"""
Plays Wordle in the browser automatically: types the guesses chosen by wordle_engine
and feeds the tile colors back to the engine.

Supported sites:
    nyt        - the daily NYT Wordle (one puzzle per day)
    unlimited  - wordleunlimited.org (a new random word every game)

Usage:
    g++ -std=c++17 -O3 -march=native wordle_engine.cpp -o wordle_engine
    .venv/bin/python auto_wordle.py [--site nyt|unlimited] [--games N] [--headless]
"""

import argparse
import subprocess
import sys
import time
from pathlib import Path

from playwright.sync_api import Error as PlaywrightError
from playwright.sync_api import sync_playwright

PROJECT_DIR = Path(__file__).resolve().parent
ENGINE = PROJECT_DIR / "wordle_engine"
STATE_TO_DIGIT = {"absent": "0", "present": "1", "correct": "2"}
MAX_ROWS = 6
ENTER_KEY = "↵"
BACKSPACE_KEY = "←"


class Engine:
    def __init__(self):
        self.proc = subprocess.Popen([str(ENGINE)], cwd=PROJECT_DIR, text=True,
                                     stdin=subprocess.PIPE, stdout=subprocess.PIPE)

    def read(self):
        line = self.proc.stdout.readline().strip()
        if not line:
            raise RuntimeError("engine exited unexpectedly")
        return line

    def send(self, message):
        self.proc.stdin.write(message + "\n")
        self.proc.stdin.flush()

    def next_guess(self):
        line = self.read()
        if not line.startswith("GUESS "):
            raise RuntimeError(f"engine: {line}")
        return line.split()[1]

    def close(self):
        self.proc.kill()


class NYTWordle:
    url = "https://www.nytimes.com/games/wordle/index.html"
    board_selector = '[aria-label="Row 1"] [data-testid="tile"]'
    # anything covering the board, the help dialog pops up a moment after the board appears
    blocker_selector = "dialog"
    # screens shown before the board, in the order they usually appear
    intro_buttons = ['[data-testid="Play"]',
                     'button:has-text("Continue to Wordle")',
                     'dialog button[aria-label="Close"]']
    # data-key is stable, while aria-label changes once a letter gets a color ("add s correct")
    key_selector = '[aria-label="Keyboard"] button[data-key="{key}"]'
    invalid_selector = '[class*="Toast-module_toast"]:has-text("Not in word list")'
    unlimited = False

    def row_tiles(self, page, row):
        return page.locator(f'[aria-label="Row {row}"] [data-testid="tile"]')

    def start_next_game(self, page):
        raise RuntimeError("NYT Wordle has only one puzzle per day")


class WordleUnlimited:
    url = "https://wordleunlimited.org/"
    board_selector = "game-row"
    blocker_selector = "game-modal[open]"
    intro_buttons = ["game-modal[open] .close-icon"]
    key_selector = 'button[data-key="{key}"]'
    invalid_selector = 'game-toast[text="Not in word list"]'
    unlimited = True

    def row_tiles(self, page, row):
        return page.locator("game-row").nth(row - 1).locator(".tile")

    def start_next_game(self, page, timeout_s=15):
        # the stats dialog with "Play again!" shows up a few seconds after a game ends
        page.locator("#refresh-button").click(timeout=timeout_s * 1000)
        deadline = time.time() + timeout_s
        while time.time() < deadline:
            if all(state == "empty" for state, _ in read_row(page, self, 1)) \
                    and not page.locator(self.blocker_selector).is_visible():
                return
            page.wait_for_timeout(200)
        raise RuntimeError("the board did not reset for a new game")


SITES = {"nyt": NYTWordle, "unlimited": WordleUnlimited}


def open_game(page, site, timeout_s=30):
    page.goto(site.url, wait_until="domcontentloaded")
    board = page.locator(site.board_selector).first
    blocker = page.locator(site.blocker_selector)
    deadline = time.time() + timeout_s
    clear_since = None
    while time.time() < deadline:
        # popups can show up a moment after the board, so require the board to stay clear for a while
        if board.is_visible() and not blocker.is_visible():
            clear_since = clear_since or time.time()
            if time.time() - clear_since > 2:
                return
        else:
            clear_since = None
        for selector in site.intro_buttons:
            button = page.locator(selector).first
            if button.is_visible():
                try:
                    button.click(timeout=2000)
                except PlaywrightError:
                    pass  # the page re-rendered under us, try again next round
                break
        page.wait_for_timeout(300)
    raise RuntimeError("could not get past the Wordle start screens")


def read_row(page, site, row):
    return site.row_tiles(page, row).evaluate_all(
        "tiles => tiles.map(t => [t.dataset.state, t.dataset.animation])")


def press(page, site, key):
    # click the on-screen keyboard: physical key presses get lost when an ad steals focus
    page.locator(site.key_selector.format(key=key)).click()


def submit_guess(page, site, row, word, timeout_s=10):
    """Types the word and returns its feedback string, or None if the site rejected the word."""
    for letter in word:
        press(page, site, letter)
    press(page, site, ENTER_KEY)

    deadline = time.time() + timeout_s
    while time.time() < deadline:
        tiles = read_row(page, site, row)
        if all(state in STATE_TO_DIGIT and anim == "idle" for state, anim in tiles):
            page.wait_for_timeout(300)  # let the reveal finish before typing again
            return "".join(STATE_TO_DIGIT[state] for state, _ in tiles)
        if page.locator(site.invalid_selector).count():
            for _ in word:
                press(page, site, BACKSPACE_KEY)
            page.wait_for_timeout(1500)  # wait for the toast to clear
            return None
        page.wait_for_timeout(100)
    raise RuntimeError(f"timed out waiting for row {row} to reveal")


def play(page, site):
    """Plays one game, returns the number of guesses used or None if it ran out of guesses."""
    engine = Engine()
    try:
        row = 1
        guess = engine.next_guess()
        while row <= MAX_ROWS:
            print(f"Guess {row}: {guess}")
            result = submit_guess(page, site, row, guess)
            if result is None:
                print(f"  '{guess}' was rejected by the site, picking another word")
                engine.send("INVALID")
                guess = engine.next_guess()
                continue

            print(f"  feedback: {result}")
            engine.send(result)
            if result == "22222":
                print(f"Solved in {row} guesses: {guess}")
                return row
            row += 1
            if row <= MAX_ROWS:
                guess = engine.next_guess()
        print("Out of guesses")
        return None
    finally:
        engine.close()


def print_stats(results):
    wins = [n for n in results if n is not None]
    print(f"\nWon {len(wins)}/{len(results)} games")
    if wins:
        print(f"Average number of guesses is: {sum(wins) / len(wins):.3f}")
        print(f"Min number of guesses is: {min(wins)}")
        print(f"Max number of guesses is: {max(wins)}")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--site", choices=SITES, default="nyt", help="which Wordle site to play (default: nyt)")
    parser.add_argument("--games", type=int, default=1, help="number of games to play, unlimited site only")
    parser.add_argument("--headless", action="store_true", help="run without showing the browser window")
    args = parser.parse_args()

    site = SITES[args.site]()
    if args.games < 1:
        parser.error("--games must be at least 1")
    if args.games > 1 and not site.unlimited:
        parser.error(f"--games only works with --site unlimited, {args.site} has one puzzle per day")
    if not ENGINE.exists():
        sys.exit(f"{ENGINE.name} not found, compile it first:\n"
                 "  g++ -std=c++17 -O3 -march=native wordle_engine.cpp -o wordle_engine")

    with sync_playwright() as p:
        browser = p.chromium.launch(headless=args.headless)
        page = browser.new_page()
        results = []
        try:
            open_game(page, site)
            for game in range(1, args.games + 1):
                if args.games > 1:
                    print(f"\n=== Game {game}/{args.games} ===")
                if game > 1:
                    site.start_next_game(page)
                results.append(play(page, site))
            if args.games > 1:
                print_stats(results)
            if not args.headless:
                input("Press Enter to close the browser...")
        except KeyboardInterrupt:
            if results:
                print_stats(results)
        except Exception:
            page.screenshot(path=PROJECT_DIR / "wordle_error.png")
            print("Saved a screenshot of the page to wordle_error.png", file=sys.stderr)
            raise
        finally:
            browser.close()


if __name__ == "__main__":
    main()
