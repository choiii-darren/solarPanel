from sun import sunPosition
import numpy as np


def find_optimal_tilt(lat, lon, month):
    """
    Function to find the optimal vertical tilt angle of a single axis tracking solar panel for each month.

    Parameters:
    lat (float): Latitude of the location.
    lon (float): Longitude of the location.
    month (int): Month number (1-12).

    Returns:
    float: The optimal vertical tilt angle for the month.
    """

    # Determine the day of the month to use for calculations (15th is approximately middle)
    day = 15

    # Array of times to check for solar noon (every 10 minutes from 6:00 to 18:00)
    times = np.arange(6, 18, 1 / 6)  # 1/6 of an hour is 10 minutes

    # Calculate the sun's position at each of these times
    elevations = [sunPosition(2023, month, day, time, lat, lon)[1] for time in times]

    # Find the maximum elevation, which occurs at solar noon
    max_elevation = max(elevations)

    # The optimal tilt angle is 90 minus the maximum elevation
    optimal_tilt = 90 - max_elevation

    return optimal_tilt


# Get user's latitude and longitude
lat = float(input("Enter your latitude: "))
lon = float(input("Enter your longitude: "))

# Iterate over all months and print the optimal tilt for each
for month in range(1, 13):
    tilt = find_optimal_tilt(lat, lon, month)
    print(f'The optimal vertical tilt angle for month {month} is {tilt:.2f} degrees.')