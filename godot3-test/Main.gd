extends Node

onready var label : RichTextLabel = $Label

onready var planet : Spatial = $planet

const SPEED = 0.10

func _ready():
	pass

var last_key = ""

func _input(event):
	if event is InputEventKey and event.pressed and not event.echo:
		label.text = "Key pressed: " + OS.get_scancode_string(event.scancode)
	elif event is InputEventJoypadButton and event.is_pressed():
		var button_name = Input.get_joy_button_string(event.button_index)
		label.text = "Gamepad pressed: " + button_name
	elif event is InputEventJoypadMotion:
		if event.axis == JOY_AXIS_0:
			planet.rotation.x = lerp(PI, -PI, (event.axis_value + 1) / 2)
		if event.axis == JOY_AXIS_1:
			planet.rotation.z = lerp(PI, -PI, (event.axis_value + 1) / 2)

