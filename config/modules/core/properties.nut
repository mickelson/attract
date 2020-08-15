/**************************************
  Core Module - Properties
  Keil Miller Jr
	liquid8d for set_properties
**************************************/

::match_aspect <- function(opts = {}) {
	try {
		assert(opts.len() == 3)

		// return height
		if ("width" in opts) return (opts.aspect_height * opts.width) / opts.aspect_width

		// return width
		else if ("height" in opts) return (opts.aspect_width * opts.height) / opts.aspect_height

		// slots not valid options
		else throw ""
	}
	catch(e) return false
}

::set_properties <- function(object, properties) {
	foreach(key, value in properties) {
		try {
			switch (key) {
				case "rgb":
					object.set_rgb(value[0], value[1], value[2])
					if (value.len() > 3) object.alpha = value[3]
					break
				case "bg_rgb":
					object.set_bg_rgb(value[0], value[1], value[2])
					if (value.len() > 3) object.bg_alpha = value[3]
					break
				case "sel_rgb":
					object.set_sel_rgb(value[0], value[1], value[2])
					if (value.len() > 3) object.sel_alpha = value[3]
					break
				case "selbg_rgb":
					object.set_selbg_rgb(value[0], value[1], value[2])
					if (value.len() > 3) object.selbg_alpha = value[3]
					break
				default:
					object[key] = value
			}
		}
		catch(e) continue
	}
}

::shade_object <- function(object, percent) {
	try {
		assert(percent >= 0 && percent <= 100)
		object.red = (percent / 100.0) * 255.0
		object.green = (percent / 100.0) * 255.0
		object.blue = (percent / 100.0) * 255.0
	}
	catch(e) return false
}

// Aliases

::set_props <- set_properties
::shade_obj <- shade_object
