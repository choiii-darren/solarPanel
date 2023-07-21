from sun import sunPosition
import numpy as np

# Define the increment angle
INCREMENT_ANGLE = 360 / 13


def find_optimal_tilt(lat, lon, month):
    """
    Function to find the optimal vertical tilt angle of a single axis tracking solar panel for each month.

    Parameters:
    lat (float): Latitude of the location.
    lon (float): Longitude of the location.
    month (int): Month number (1-12).

    Returns:
    float: The optimal vertical tilt angle for the month.
    int: The number of increments to adjust the tilt.
    str: The direction to point the panel.
    """

    day = 15
    times = np.arange(0, 24)
    elevations = [sunPosition(2023, month, day, time, 0, 0, lat, lon)[1] for time in times]
    max_elevation = max(elevations)
    optimal_tilt = 90 - max_elevation

    # Calculate the number of increments
    increments = round(optimal_tilt / INCREMENT_ANGLE)

    # Determine the direction to point the panel
    direction = "north" if lat < 0 else "south"  # Point south in the northern hemisphere, and north in the southern hemisphere

    return optimal_tilt, increments, direction


# Get user's latitude and longitude
lat = float(input("Enter your latitude: "))
lon = float(input("Enter your longitude: "))

# Iterate over all months and print the optimal tilt for each
for month in range(1, 13):
    tilt, increments, direction = find_optimal_tilt(lat, lon, month)
    print(
        f'The optimal vertical tilt angle for month {month} is {tilt:.2f} degrees. Tilt down by {increments} increments. Point the panel to the {direction}.')
