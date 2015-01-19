def arduino_map(x, in_min, in_max, out_min, out_max):
        return (x - in_min) * (out_max - out_min) // (in_max - in_min) + out_min

def arduino_constrain(val, minVal, maxVal):
	if val > maxVal:
		return maxVal
		
	if val < minVal:
		return minVal
	
	return val
