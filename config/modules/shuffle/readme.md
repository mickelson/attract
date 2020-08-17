# Shuffle Module

[Keil Miller Jr](mailto:keilmillerjr@outlook.com)

## DESCRIPTION:

Shuffle module takes an array of objects and uses them as a form of list navigation.

This module is meant to be simple. It does not handle presentation of object, nor does it handle object animation. If you are looking for an animated list, please look at the conveyor module.

## Supported Objects

* artwork
* image
* text
* preserveImage
* preserveArt

## Constructor Options

| Key | Type | Required|
|:---|:---:|:---:|
| `deselected_slots_visible` | boolean | NO |
| `reset_new_list` | boolean | NO |
| `loop_list` | boolean | NO |
| `slots` | array | YES |

## Protected Functions

These methods, you should not have to overridden or extend unless you are modifying functionality of shuffle.

* `_refresh_deselected_slot(slot)`
* `_refresh_selected_slot(slot)`
* `_refresh_slots()`

## Usage

From within your layout, you will load the Shuffle module. Shuffle keeps presentation and logic separate. It is up to you to create your objects and apply properties to them. You will then create an instance of the class.

```squirrel
// Load the Shuffle module
fe.load_module("shuffle")

// Create your objects
local list = []
	list.push(fe.add_text("[Title]", 0, 0, fe.layout.width/3, fe.layout.height/26))
	list.push(fe.add_text("[Title]", fe.layout.width/3, 0, fe.layout.width/3, fe.layout.height/26))
	list.push(fe.add_text("[Title]", (fe.layout.width/3)*2, 0, fe.layout.width/3, fe.layout.height/26))

// Create an instance of the ShuffleList class
local shuffle_list = Shuffle({slots = list})

```

#### Extending The Shuffle Class

This example will extend the Shuffle class, make a selected slot bold, and deselected slots regular.

```squirrel
// Load the Shuffle module
fe.load_module("shuffle")

// Create your objects
local list = []
	list.push(fe.add_text("[Title]", 0, 0, fe.layout.width/3, fe.layout.height/26))
	list.push(fe.add_text("[Title]", fe.layout.width/3, 0, fe.layout.width/3, fe.layout.height/26))
	list.push(fe.add_text("[Title]", (fe.layout.width/3)*2, 0, fe.layout.width/3, fe.layout.height/26))

// extend the shuffle class
class ShuffleList extends Shuffle {
	// overridden method
	function _refresh_selected_slot(slot) {
		slot.red = 255
		slot.green = 0
		slot.blue = 0
		slot.alpha = 255
	}

	// overridden method
	function _refresh_deselected_slot(slot) {
		slot.red = 255
		slot.green = 255
		slot.blue = 255
		slot.alpha = 255/2
	}

	// overridden method
	function _refresh_slots() {
		// call the base method
		base._refresh_slots()
	}
}

// Create an instance of the ShuffleList class
local shuffle_list = ShuffleList({slots = list})

```
