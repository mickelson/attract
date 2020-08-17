/**************************************
  Shuffle Module
  Keil Miller Jr
**************************************/

fe.load_module("core")

class Shuffle {
	_deselected_slots_visible = null
	_loop_list = null
	_reset_new_list = null
	_selected_slot = null
	_skip_update_selected_slot = null
	_slots = null

	constructor(opts) {
		this._selected_slot = 0
		this._skip_update_selected_slot = false

		if ("deselected_slots_visible" in opts) this._deselected_slots_visible = to_boolean(opts.deselected_slots_visible)
		else this._deselected_slots_visible = true

		if ("reset_new_list" in opts) this._reset_new_list = to_boolean(opts.reset_new_list)
		else this._reset_new_list = true

		if ("loop_list" in opts) this._loop_list = to_boolean(opts.loop_list)
		else this._loop_list = true

		if (__validate_slots(opts.slots)) {
			this._slots = opts.slots

			fe.add_signal_handler(this, "__signal")
			fe.add_transition_callback(this, "__transition")
		}
		else log.error("Invalid slots passed to Shuffle constructor. Disabling signal handler and transition callback.")
	}

	// PUBLIC

	// PROTECTED

	function _refresh_deselected_slot(slot) {
		this._deselected_slots_visible ? slot.visible = true : slot.visible = false
	}

	function _refresh_selected_slot(slot) {
		slot.visible = true
	}

	function _refresh_slots() {
		for (local i = 0; i < this._slots.len(); i++) {
			i == this._selected_slot ? _refresh_selected_slot(this._slots[i]) : _refresh_deselected_slot(this._slots[i])
		}
	}

	// PRIVATE

	function __signal(signal_str) {
		switch(signal_str) {
			// ignore signals at start or end of list when looping is false
			case "prev_game":
				if (this._loop_list == false && fe.list.index == 0) return true
				break
			case "next_game":
				if (this._loop_list == false && fe.list.index == fe.list.size-1) return true
				break
			case "random_game":
			case "prev_letter":
			case "next_letter":
			case "add_favourite":
			case "prev_favourite":
			case "next_favorite":
				this._skip_update_selected_slot = true
				break
		}

		return false
	}

	function __transition(ttype, var, ttime) {
		if (ttype == Transition.ToNewList || ttype == Transition.FromOldSelection) {
			if (this._skip_update_selected_slot) this._skip_update_selected_slot = false
			else if (ttype == Transition.FromOldSelection) __update_selected_slot(var)
			else if (ttype == Transition.ToNewList && this._reset_new_list) {
				this._selected_slot = 0
				fe.list.index = 0
			}

			__update_index_offsets()
			_refresh_slots()
		}

		return false
	}

	function __update_index_offsets() {
		for (local i = 0; i < this._slots.len(); i++) {
			try { this._slots[i].index_offset = -(this._selected_slot - i) } catch(e) {}
			try { this._slots[i].art.index_offset = -(this._selected_slot - i) } catch(e) {}
		}
	}

	function __update_selected_slot(position) {
		// increase selected slot
		if (position < 0 && this._selected_slot < (this._slots.len() - 1)) this._selected_slot++
		// decrease selected_slot
		else if (position > 0 && this._selected_slot > 0) this._selected_slot--
	}

	function __validate_slots(slots) {
		try {
			foreach (slot in slots) {
				assert(typeof slot.index_offset == "integer" || typeof slot.art.index_offset == "integer")
			}

			return true
		}
		catch(e) return false
	}
}
