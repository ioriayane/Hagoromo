---
title: Getting started
weight: 10
description: This is a multi-column Bluesky client.
---

# Getting started

## Download

Download the zip file from the [top page](/).

Please select the one that best fits the environment in which you will use it. The file name is as follows:

- Windows : `hagoromo_x.y.z_windows.zip`
- Linux : `hagoromo_x.y.z_linux.zip`
- Mac : `hagoromo_x.y.z_mac.zip`

## Install

Unzip the downloaded zip file to any folder and execute the following file.

- Windows : `Hagoromo.exe`
- Linux : `Hagoromo.sh`
- Mac : `Hagoromo.app`

For Mac, please refer to [Open a Mac app from an unidentified developer](https://support.apple.com/guide/mac-help/open-a-mac-app-from-an-unidentified-developer-mh40616/mac).

## Add Accounts

{{< figure src="../images/add_account.en.png" >}}

1. Open the Manage Accounts dialog from the sidebar.
2. Press the Add button.

{{< figure src="../images/login.en.png" >}}

1. Set the following:
   - Service  
     Input `https://bsky.social`.  
     Basically no change is required, but other PDS can be used.
   - ID  
     Input oen of the following:
     - Handle
     - E-mail
     - DID
   - Password  
     Input the application password you created on the [official site](https://bsky.app/settings/app-passwords).  
     Do not use real passwords.
2. Press the `Login` button.

## Add columns

{{< figure src="../images/add_column.en.png" >}}

1. Select the account
2. Choose the item you want to add from the column types
3. Press the Add button

Display the following column types:

- Default Feeds
  - Displays `Following` and `Notifications`.
- My Feeds
  - Displays the saved custom feed(not `Pin to Home`).
- My Lists
  - Displays my lists (not a list to mute or block).

## Post

{{< figure src="../images/post.en.png" >}}

1. Press the button marked with a pencil (or `n` key).
2. Please input your wonderful event (up to 300 charactors).
3. Press the `POST` button.

## Adjust scroll velocity

The scrolling velocity of the columns may be too fast or too slow for your environment, but an adjustment function is provided.

{{< figure src="../images/scroll_velocity.en.png" width="90%" >}}

- If too fast: Adjust to the left
- If too slow: Adjust to the right

The default value is 2500.

- Scroll velocity
  - Sets the initial speed when flicking with the mouse wheel or touchpad, etc.
    - Adjust to the right (faster): If it is too slow
    - Adjust to the left (slower): If it is too fast
  - Default: 2500
- Scroll deceleration
  - Sets how easily the scroll stops after it has started moving.
    - Adjust to the right (higher): Stops immediately
    - Adjust to low (left): Skips a lot
  - Default: 10000
- Test button
  - After adjusting the slide bar, press this button to refresh the sample list.
  - Adjusting the slide bar alone will not have any effect.

**Note**

After making adjustments, press the `OK` button in the settings dialog and then restart Hagoromo.
