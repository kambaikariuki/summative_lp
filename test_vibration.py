import vibration

# Sample vibration data
data = [0.5, 1.2, -0.7, 2.3, 1.8, -1.1]

print("Data:", data)

print("Peak-to-Peak:", vibration.peak_to_peak(data))
print("RMS:", vibration.rms(data))
print("Std Dev:", vibration.std_dev(data))
print("Above Threshold (1.0):", vibration.above_threshold(data, 1.0))
print("Summary:", vibration.summary(data))

# Edge case: empty list
try:
    print(vibration.rms([]))
except Exception as e:
    print("Error (empty input):", e)

# Invalid input
try:
    print(vibration.rms([1, 2, "bad"]))
except Exception as e:
    print("Error (invalid type):", e)
