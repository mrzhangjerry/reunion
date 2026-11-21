Reunion Simulation
A C program that estimates the probability of two people meeting again after they walk away from each other in a city.
How the code works
The program represents a 20 km × 20 km city as a 200 × 200 grid. Each cell is 100 m × 100 m, and a Point struct stores each person's x and y coordinates.

At the start of each trial, both people occupy the same cell. They take their first steps in opposite directions, then independently choose a random direction at each step. If a move would leave the grid, the person picks another direction. Each person takes 40 steps per day, covering 4 km.

After both people move, the program compares their coordinates. If both coordinates match, it records a reunion. A flag tracks whether the pair has already met in that trial, so the first reunion is counted only once. The simulation continues after a reunion to record later encounters.
Output
By default, the program runs 20,000 trials over five simulated years. It writes two CSV files:
- minute_probability.csv reports the probability of occupying the same cell at each step, the probability of a first reunion at that step, and the cumulative probability of having met by then.
- annual_probability.csv reports the probability of meeting at least once during each year, having a first reunion during that year, and having met at least once by year-end.
