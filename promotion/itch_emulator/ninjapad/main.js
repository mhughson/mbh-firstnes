ninjapad.emulator = null;
ninjapad.jQElement = null;
ninjapad.initialize = function() {
    ninjapad.jQElement = {
        gamepad:        $("#GAMEPAD"),
        controller:     $("#GAMEPAD-BUTTONS"),
        analogSwitch:   $("#analogSwitch"),
        menu:           $("#menu"),
        upload:         $("#upload"),
        analogStick:    $("#ANALOG_STICK"),
        analog:         $("#ANALOG"),
        dpad:           $("#DPAD"),
        osd:            $("#OSD"),
        screen:         $("#" + SCREEN),
    };

    // Page setup
    ninjapad.layout.setPageLayout();

    // Assign function calls to touch events
    ninjapad.utils.assign(ninjapad.gamepad.toggleMenu, "menu", "start", "end");
    ninjapad.utils.assign(ninjapad.gamepad.analogSwitch, "analogSwitch", "start", "end");
    ninjapad.utils.assign(ninjapad.gamepad.buttonPress, "GAMEPAD-BUTTONS", "start", "move", "end");
    ninjapad.utils.assign(ninjapad.gamepad.analogTouch, "ANALOG_STICK", "start", "move", "end");
    ninjapad.utils.assign(ninjapad.gamepad.toggleFullScreen, SCREEN, "end");
    ninjapad.utils.assign(null, "GAMEPAD");
};

$(document).ready(function() {
    DEBUG && console.log("Document ready event");

    // Load emulator
    ninjapad.emulator = ninjapad.interface[EMULATOR];

    // Pause on loss of focus
    $(window).blur(function() {
        !ninjapad.pause.state.isEmulationPaused &&
        ninjapad.utils.isMobileDevice() &&
        ninjapad.pause.pauseEmulation();
    });

    // Reload layout on orientation change
    $(window).resize(function() {
        DEBUG && console.log("Window resize event");
        ninjapad.initialize();
    });

    // Use ESC key to open the menu
    $(window).keyup(function(e) {
      if (e.code == "Escape") ninjapad.menu.toggleMenu();
    });

    // Load a ROM and setup the page layout
    ninjapad.emulator.initialize();
    ninjapad.initialize();
});
