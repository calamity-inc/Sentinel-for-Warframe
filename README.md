# Sentinel

Sentinel is an external helper and overlay for Warframe.

## Features

Sentinel GUI:
- Invasions Helper: Shows rare invasion rewards and indicates if they're already owned.

In-game overlay:
- Squad Overlay: Shows additional info about your squad members (Is host, IP-based location, ISP, is using VPN)
- Rescue Helper: Tells you which cell the hostage is in if you're the host.
- Duviri Tarot: Tells you what warframes and weapons you will be able to choose in the cave. This is basically the same as having rank 4 or higher of the Opportunity intrinsic.

## TODO

Additions:
- Bounty Helper: Show if Narmer bounties are available and if you own the respective Caliban blueprint already

Improvements:
- Duviri Tarot: Make it a toggle since the detection for if the Duviri navigation is opened is really unreliable
- Rescue Helper: Show cell name so you don't need to cross-reference the code comment
- Rescue Helper: Give more time for end-user to screenshot/write down cell name after hostage was found (in an unknown cell)

## A note on inventory data

Currently, Sentinel's only way to read your inventory data is by reading [AlecaFrame](https://www.alecaframe.com/)'s cached data, so you need to have it installed.

However, it is fine to use Sentinel without inventory data; it will just assume that you don't own anything, meaning "Bounty Helper" and "Invasions Helper" can't indicate if an item is already owned.
