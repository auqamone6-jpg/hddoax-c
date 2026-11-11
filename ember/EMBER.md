# Ember

An iPhone app that runs an emulated iPhone 11 with iOS 14, full screen.

Ember is a modified copy of [Inferno for iPhone](https://github.com/MakrSas/Inferno-iOS)
by MakrSas, which is a front end for [Inferno](https://github.com/ChefKissInc/Inferno),
the emulator by Visual Ehrmanntraut and the Inferno team at ChefKiss. The emulator
and everything that talks to it are theirs. Ember is unofficial and is not
affiliated with or endorsed by either.

Licence: GPL-3.0, the same as the app it is built from. See `LICENSE`. The
original `README.md` is kept as it was and still describes how everything works.

## What Ember changes

- **Enable JIT button.** One tap hands this app to StikDebug with the right
  script, then the app keeps checking until JIT is on.
- **Start button on the first screen** once the disk is in place and JIT is on.
- **Full screen as soon as the machine starts.** Settings has a switch for it.
- **Smooth mode** in Settings → Translator: the fastest known-good settings in
  one tap.
- **Open the folder in Files** button on the setup screen.
- **`build-ipa-on-mac.sh`**: one command on a Mac that builds the `.ipa`.
- App name and bundle identifier.

None of these changes has been compiled or run yet.

## What you need

- A guest image (the "disk") that you build once on a computer by ChefKiss's
  guides, linked from `README.md`. No Apple firmware, image or key is included
  here.
- StikDebug, LocalDevVPN and a pairing file, for JIT.

## Building

On a Mac with Xcode and Homebrew, from this folder:

    ./build-ipa-on-mac.sh

Or open `Ember.xcodeproj` in Xcode and press Build: it runs the same script,
and the log is in Xcode's report navigator.

The result is `Inferno.ipa` in this folder (the file keeps the original name;
the app installs as Ember). The project does not install the app on a phone
or run it; that is still done with a sideloading tool.

## Trying it in the iOS Simulator (experimental)

On a Mac, Apple Silicon or Intel:

    ./run-in-simulator.sh

or choose the **Ember (Simulator)** scheme in `Ember.xcodeproj` and press
Build. It builds the emulator and the app for the Simulator, installs the app
in the booted Simulator (it boots an iPhone 17 Pro if none is running), opens
it, and prints the folder the disk goes in.

The emulator's authors never built it for the Simulator, and this has not been
run. It may stop while building the emulator, or when the machine starts. The
Simulator also says little about a real phone: JIT, the memory limit and speed
all behave differently there.

## Using

1. Install the `.ipa` and StikDebug with a sideloading tool.
2. Open Ember once, then copy `InfernoData` and `AppleSEPROM-Cebu-B1` into its
   folder: Files → On My iPhone → Ember, or drag them in through Finder.
3. Tap **Enable JIT**, then **Start**.
