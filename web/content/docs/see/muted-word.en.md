---
title: Muted word
weight: 80
---

# Muted word

You can hide posts using pre-registered words.

The registration of words can be shared with official applications. However, the criteria for being hidden is different from official and other third-party clients.

## Mute Criteria

### Post body text

- When Japanese is included in the post's language setting
  - Mute the post when the post body contains a string of characters registered as a mute word.
- When the post language setting does not include Japanese
  - Mute when the post body is divided by spaces and a string that exactly matches the mute word is found.

### Tag

- Mute when an exact match is found with the hashtag string embedded inside the post.

## Register words

### From hashtag

You can register mute words from hashtags set in the post.

{{< figure src="../images/muted-word-from-hashtag.en.png" width="100%" >}}

1. Right-click on the hashtag
2. Select "Mute #XXX" from the context menu
3. The Edit Mute Words dialog box will open.
4. Choose from the following options to check for mute
   - Text and tags: The text and the hashtag information behind the hashtag.
   - Tags: only the hashtag information you have on the back
5. Press the "Add/Update" button
6. Press the "Save" button


#### Supplement

- You can delete a word by clicking on the trash button.
- Clicking on a registered word copies the word to the edit box.
- Attempting to register an existing word will overwrite the existing settings.
  - In other words, you can change the words to be muted or checked.
  - You cannot change the word itself. You can add and remove words.
- If you add `#` to the first letter, it will be removed and registered.
  - This is because hashtags are internally embedded in the post without the `#`.
- If you change the mute word in another client while working on the addition, it will be overwritten with the content in Hagoromo.
- The button to the right of the account name will reload the settings stored on the server.
  - The content you are editing will be lost. Please note.

### From Account Management

You can edit the settings for each account.

{{< figure src="../images/muted-word-from-account.en.png" width="100%" >}}

1. Open Account Management Dialog
2. Select "Muted words and tags" from the menu of the account you wish to edit
3. As well as registration from the hashtag
