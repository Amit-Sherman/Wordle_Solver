# Wordle_Solver
This projects feature a wordle solver, utilizing information theory

Suggested compilation line: g++ -O3 -march=native cheatingWordle.cpp -o wordle_cheat

## Auto-play Wordle in the browser
`auto_wordle.py` opens Wordle in a Chromium window, types the solver's guesses and reads the tile colors back as feedback.
It supports the daily NYT Wordle (`--site nyt`, the default) and wordleunlimited.org (`--site unlimited`).

Setup (once):
```
g++ -std=c++17 -O3 -march=native wordle_engine.cpp -o wordle_engine
python3 -m venv .venv
.venv/bin/pip install -r requirements.txt
.venv/bin/python -m playwright install chromium
```

Run:
```
.venv/bin/python auto_wordle.py            # visible browser window
.venv/bin/python auto_wordle.py --headless # no window
.venv/bin/python auto_wordle.py --site unlimited --games 20  # 20 games in a row, then guess stats
```
The browser is a fresh profile, so results are not saved to an NYT account.
