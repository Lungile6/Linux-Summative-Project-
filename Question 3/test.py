import numpy as np
import temp_stats

# Sample temperature data
temperature_readings = np.array([20.5, 21.0, 20.0, 22.0, 21.5, 19.5, 20.8, 21.2, 20.7, 21.1], dtype=np.float32)

print(f"Temperature Readings: {temperature_readings}")

# Test the C extension functions
min_val = temp_stats.min_temp(temperature_readings)
max_val = temp_stats.max_temp(temperature_readings)
avg_val = temp_stats.avg_temp(temperature_readings)
variance_val = temp_stats.variance_temp(temperature_readings)
count_val = temp_stats.count_readings(temperature_readings)

print(f"Minimum Temperature: {min_val}")
print(f"Maximum Temperature: {max_val}")
print(f"Average Temperature: {avg_val}")
print(f"Variance of Temperature: {variance_val}")
print(f"Number of Readings: {count_val}")

# Test with an empty array
empty_readings = np.array([], dtype=np.float32)
print("\nTesting with empty array:")
try:
    temp_stats.min_temp(empty_readings)
except ValueError as e:
    print(f"Error for min_temp with empty array: {e}")
try:
    temp_stats.max_temp(empty_readings)
except ValueError as e:
    print(f"Error for max_temp with empty array: {e}")
try:
    temp_stats.avg_temp(empty_readings)
except ValueError as e:
    print(f"Error for avg_temp with empty array: {e}")
try:
    temp_stats.variance_temp(empty_readings)
except ValueError as e:
    print(f"Error for variance_temp with empty array: {e}")

print(f"Number of Readings (empty array): {temp_stats.count_readings(empty_readings)}")

# Test with a single element array for variance
single_reading = np.array([25.0], dtype=np.float32)
print("\nTesting with single element array:")
print(f"Temperature Reading: {single_reading}")
print(f"Minimum Temperature: {temp_stats.min_temp(single_reading)}")
print(f"Maximum Temperature: {temp_stats.max_temp(single_reading)}")
print(f"Average Temperature: {temp_stats.avg_temp(single_reading)}")
try:
    temp_stats.variance_temp(single_reading)
except ValueError as e:
    print(f"Error for variance_temp with single element array: {e}")
print(f"Number of Readings: {temp_stats.count_readings(single_reading)}")
