/**************************************
  Core Module - Logger
  Keil Miller Jr
**************************************/

class Logger {
  categories = null
  level = null

	constructor() {
    categories = [
      "debug", // Low-level information for developers.
      "info",  // Generic (useful) information about system operation.
      "warn",  // A warning.
      "error", // A handleable error condition.
      "fatal", // An unhandleable error that results in a program crash.
    ]

		set_level(1)
	}

  // Public

	function set_level(severity) { // Filter messages based on severity
		if (typeof severity == "integer" && severity >= 0 && severity <= 4) level = severity
    else if (typeof severity == "string" && categories.find(severity.tolower())) level = categories.find(severity.tolower())
    else action_handler(categories[2], "Log level set is not valid.")
	}

  function debug(message) { action_handler(categories[0], message) }
  function info(message) { action_handler(categories[1], message) }
  function warn(message) { action_handler(categories[2], message) }
  function error(message) { action_handler(categories[3], message) }
  function fatal(message) { action_handler(categories[4], message) }

  // Private

	function action_handler(severity, message) {
    if (categories.find(severity) < level) return

    local stack = getstackinfos(3)
    print("[" + stack.src + ":" + stack.line + "]: (" + severity.toupper() + ") " + message + "\n")
    stack = null
	}
}
::log <- Logger()
