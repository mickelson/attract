/**************************************
  Core Module
  Keil Miller Jr
**************************************/

::explode <- function(string, separators = ",.-/|;") {
	try { return split(string, separators) }
	catch (e) return false
}

::implode <- function(array, separator = "") {
	try {
		local output = ""
		foreach (value in array) { output += value + separator }
		return output
	}
	catch (e) return false
}

::in_range <- function(val, low, high) {
	try {
		if (val >= low && val <= high) return true
		else return false
	}
	catch (e) return false
}

::mean <- function(array) {
	try {
		local sum = 0.0
		foreach (value in array) sum += value
		return sum / array.len()
	}
	catch(e) return false
}

::median <- function(array) {
	try {
		array.sort()
		if (array.len() % 2 == 0) return (array[array.len() / 2 - 1] + array[array.len() / 2]) / 2.0 // Return value for mean of two middle numbers
		else return array[(array.len() - 1) / 2] // Return value for middle number
	}
	catch(e) return false
}

::mode <- function(array) {
	try {
		local frequency = {}
		foreach (value in array)
			value in frequency ? frequency[value] += 1 : frequency[value] <- 1

		local maximum = []
		foreach (key, value in frequency) {
			// maximum array is empty OR value is greater than the frequency of the first key in maximum
			if (maximum.len() == 0 || value > frequency[maximum[0]])
				maximum = [key]
			// value is equal to the frequency of the first key in maximum AND the key does not exist in maximum
			else if (value == frequency[maximum[0]] && maximum.find(key) == null)
				maximum.push(key)
		}

		return maximum
	}
	catch(e) return false
}

::percentage <- function(opts) {
	try {
		// Aliases
		if ("percent" in opts && !("p" in opts)) {
			opts.p <- opts.percent
			delete opts.percent
		}
		if ("part" in opts && !("x" in opts)) {
			opts.x <- opts.part
			delete opts.part
		}
		if ("whole" in opts && !("y" in opts)) {
			opts.y <- opts.whole
			delete opts.whole
		}

		assert(opts.len() == 2)

		// part = whole * percent / 100
		if ("p" in opts && "y" in opts)
			return (opts.y.tofloat() * opts.p.tofloat()) / 100.0

		// percent = part * 100 / whole
		else if ("x" in opts && "y" in opts)
			return (opts.x.tofloat() * 100.0) / opts.y.tofloat()

		// whole = part * 100 / percent
		else if ("p" in opts && "x" in opts)
			return (opts.x.tofloat() * 100.0) / opts.p.tofloat()

		// slots not valid options
		else throw ""
	}
	catch (e) return false
}

::random_boolean <- function() {
	return random_integer(1) == 1
}

::random_integer <- function(max) {
	// Seed based on the fastrand32.py module from the python library PyRandLib.
	// https://github.com/schmouk/PyRandLib/blob/master/PyRandLib/fastrand32.py
	try {
		local seedTime = time() * 1000
		srand(((seedTime & 0xff000000) >> 24) + ((seedTime & 0x00ff0000) >>  8) + ((seedTime & 0x0000ff00) <<  8) + ((seedTime & 0x000000ff) << 24))
		return ((1.0 * rand() / RAND_MAX) * (max + 1)).tointeger()
	}
	catch (e) return false
}

::to_boolean <- function(x) {
	local data = [
		"accept",
		"checked",
		"on",
		"positive",
		"true",
		"yes",
	]

	switch(typeof x) {
		case "bool": // Squirrel has a boolean type(bool) however like C++ it considers null, 0(integer) and 0.0(float) as false, any other value is considered true.
			return x
			break
		case "string":
			if (data.find(x.tolower()) != null) return true
			break
		default:
			return false
			break
	}
}

// Aliases

::join <- implode
::per <- percentage
::rand_bool <- random_boolean
::rand_int <- random_integer
::to_bool <- to_boolean

// Include

fe.do_nut(fe.module_dir + "logger.nut")
fe.do_nut(fe.module_dir + "properties.nut")
