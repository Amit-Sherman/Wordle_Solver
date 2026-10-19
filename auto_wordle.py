"""
Plays the daily NYT Wordle automatically: types the guesses chosen by wordle_engine
into the browser and feeds the tile colors back to the engine.

Usage:
    g++ -std=c++17 -O3 -march=native wordle_engine.cpp -o wordle_engine
    .venv/bin/python auto_wordle.py [--headless]
"""

import argparse
import subprocess
import sys
import time
from pathlib import Path

from playwright.sync_api import Error as PlaywrightError
from playwright.sync_api import sync_playwright

WORDLE_URL = "https://www.nytimes.com/games/wordle/index.html"
PROJECT_DIR = Path(__file__).resolve().parent
ENGINE = PROJECT_DIR / "wordle_engine"
STATE_TO_DIGIT = {"absent": "0", "present": "1", "correct": "2"}
MAX_ROWS = 6


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


# screens shown before the board, in the order they usually appear
INTRO_BUTTONS = ['[data-testid="Play"]',
                 'button:has-text("Continue to Wordle")',
                 'dialog button[aria-label="Close"]']


def open_game(page, timeout_s=30):
    page.goto(WORDLE_URL, wait_until="domcontentloaded")
    board = page.locator('[aria-label="Row 1"] [data-testid="tile"]').first
    help_dialog = page.locator('dialog')
    deadline = time.time() + timeout_s
    clear_since = None
    while time.time() < deadline:
        # the help dialog pops up a moment after the board, so require the board to stay clear for a while
        if board.is_visible() and not help_dialog.is_visible():
            clear_since = clear_since or time.time()
            if time.time() - clear_since > 2:
                return
        else:
            clear_since = None
        for selector in INTRO_BUTTONS:
            button = page.locator(selector).first
            if button.is_visible():
                try:
                    button.click(timeout=2000)
                except PlaywrightError:
                    pass  # the page re-rendered under us, try again next round
                break
        page.wait_for_timeout(300)
    raise RuntimeError("could not get past the Wordle start screens")


def row_tiles(page, row):
    return page.eval_on_selector_all(
        f'[aria-label="Row {row}"] [data-testid="tile"]',
        "tiles => tiles.map(t => [t.dataset.state, t.dataset.animation])")


ENTER_KEY = "↵"
BACKSPACE_KEY = "←"


def press(page, key):
    # click the on-screen keyboard: physical key presses get lost when the ad banner steals focus.
    # data-key is stable, while aria-label changes once a letter gets a color ("add s correct")
    page.locator(f'[aria-label="Keyboard"] button[data-key="{key}"]').click()


def submit_guess(page, row, word, timeout_s=10):
    """Types the word and returns its feedback string, or None if the site rejected the word."""
    for letter in word:
        press(page, letter)
    press(page, ENTER_KEY)

    deadline = time.time() + timeout_s
    while time.time() < deadline:
        tiles = row_tiles(page, row)
        if all(state in STATE_TO_DIGIT and anim == "idle" for state, anim in tiles):
            page.wait_for_timeout(300)  # let the reveal finish before typing again
            return "".join(STATE_TO_DIGIT[state] for state, _ in tiles)
        if page.locator('[class*="Toast-module_toast"]:has-text("Not in word list")').count():
            for _ in word:
                press(page, BACKSPACE_KEY)
            page.wait_for_timeout(1500)  # wait for the toast to clear
            return None
        page.wait_for_timeout(100)
    raise RuntimeError(f"timed out waiting for row {row} to reveal")


def play(page, engine):
    row = 1
    guess = engine.next_guess()
    while row <= MAX_ROWS:
        print(f"Guess {row}: {guess}")
        result = submit_guess(page, row, guess)
        if result is None:
            print(f"  '{guess}' was rejected by the site, picking another word")
            engine.send("INVALID")
            guess = engine.next_guess()
            continue

        print(f"  feedback: {result}")
        engine.send(result)
        if result == "22222":
            print(f"Solved in {row} guesses: {guess}")
            return True
        row += 1
        if row <= MAX_ROWS:
            guess = engine.next_guess()
    print("Out of guesses")
    return False


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--headless", action="store_true", help="run without showing the browser window")
    args = parser.parse_args()

    if not ENGINE.exists():
        sys.exit(f"{ENGINE.name} not found, compile it first:\n"
                 "  g++ -std=c++17 -O3 -march=native wordle_engine.cpp -o wordle_engine")

    engine = Engine()
    with sync_playwright() as p:
        browser = p.chromium.launch(headless=args.headless)
        page = browser.new_page()
        try:
            open_game(page)
            play(page, engine)
            if not args.headless:
                input("Press Enter to close the browser...")
        except Exception:
            page.screenshot(path=PROJECT_DIR / "wordle_error.png")
            print("Saved a screenshot of the page to wordle_error.png", file=sys.stderr)
            raise
        finally:
            engine.close()
            browser.close()


if __name__ == "__main__":
    main()
