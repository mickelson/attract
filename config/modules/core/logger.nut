/**************************************
  Core Module - Logger
  Keil Miller Jr
**************************************/

class Logger {
  CATEGORIES = null
  _level = null

	constructor() {
    CATEGORIES = [
      "debug", // Low-level information for developers.
      "info",  // Generic (useful) information about system operation.
      "warn",  // A warning.
      "error", // A handleable error condition.
      "fatal", // An unhandleable error that results in a program crash.
    ]

		set_level(1)
	}

  // Public

  function get_categories() { return CATEGORIES }

	function set_level(severity) { // Filter messages based on severity
		if (typeof severity == "integer" && severity >= 0 && severity <= 4) this._level = severity
    else if (typeof severity == "string" && CATEGORIES.find(severity.tolower())) this._level = CATEGORIES.find(severity.tolower())
    else _action_handler(CATEGORIES[2], "Log level set is not valid.")
	}

  function debug(message) { _action_handler(CATEGORIES[0], message) }
  function info(message) { _action_handler(CATEGORIES[1], message) }
  function warn(message) { _action_handler(CATEGORIES[2], message) }
  function error(message) { _action_handler(CATEGORIES[3], message) }
  function fatal(message) { _action_handler(CATEGORIES[4], message) }

  // Protected

	function _action_handler(severity, message) {
    if (CATEGORIES.find(severity) < this._level) return

    local stack = getstackinfos(3)
    print("[" + stack.src + ":" + stack.line + "]: (" + severity.toupper() + ") " + message + "\n")
    stack = null
	}

  // Private
}
::log <- Logger()
