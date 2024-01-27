---
title: Various posts
weight: 20
---

# Various posts

## Reply

You can reply to the post.

{{< figure src="../images/post_reply.en.png" width="100%" >}}

1. The `Reply` button on the post brings up a dialog.
2. The post to which you are replying appears at the top of the post dialog.
3. Input the content of your reply.
4. Press the `POST` button.

## Quote

You can quote the post.

{{< figure src="../images/post_quote.en.png" width="100%" >}}

1. Select `Quote` from the menu displayed by the post's `Repost` button.
2. The post from which you are quoting is displayed at the bottom of the post dialog.
3. Input the post content.
4. Press the `POST` button.

## Mention

Mention the handle in the post in the format of `@handle` to make a mentions.

### Mention from Profile

Assists with mentions to accounts displaying profiles.

{{< figure src="../images/post_mention.en.png" width="100%" >}}

1. Press the post icon to display your profile.
2. Press the `...` button at the bottom right of the profile details.
3. Select `Send mention` from the menu.
4. The post dialog appears with handles inputed.

### Search for candidate handles

Assists in inputing handles in editing the post body.

{{< figure src="../images/post_mention_suggestion.en.png" width="70%" >}}

1. Enter `@` followed by a letter of the alphabet.
2. Candidate accounts will be searched and displayed.
3. Select with the up/down cursor keys.
4. Press Enter to confirm. (Input space to exit)

## Add hashtags

You can include hashtags in your post.

{{< figure src="../images/post_hashtag.en.png" width="100%" >}}

The following strings of conditions are recognized as hashtags and posted.

- Conditions
  - A string of non-whitespace characters beginning with `#`.
  - However, the next letter after `#` must be a non-numeric character.
  - Blanks before and after, line breaks, sentence beginnings, and sentence endings
- For another expression
  - `(BOL or white space charactor)#(non-numeric and non-whitespace character)(non-white space characters)(white space charactor or EOL)`
- For regular expressions
  - `(?:^|\s)(#[^\d\s]\S*)(?=\s)?`

## Add a link card

You can create a link card for a website with [OGP(Open Graph Protocol)](https://ogp.me/).

{{< figure src="../images/post_linkcard.en.png" width="100%" >}}

1. Input a URL.
2. Press the `+` button.
3. If you can get the OGP, you will see a preview of the link card.

You can remove the link card with the `trash` button.

## Attach images

You can attach images.

{{< figure src="../images/post_image_select.en.png" width="100%" >}}

1. Press the `Image` button.
2. An image selection dialog appears.
3. Select up to 4 images.
4. Press Open.
5. A preview of the image is displayed.

You can add Alt messages.

{{< figure src="../images/post_image_alt.en.png" width="100%" >}}

1. Press Preview Image.
2. The Alt message input dialog appears.
3. Input your message and press the `Add` button.
4. The `Alt` symbol appears in the image preview.

You can unattach images with the `tras` button.

## Setting labels

Can be labeled for sensitive posts. Used primarily for image postings.

{{< figure src="../images/post_labeling.en.png" width="100%" >}}

1. Press the `shield symbol` button.
2. Select the appropriate item from the menu that appears.

If the viewing user has set the display of sensitive contents to `warning`, the images will be hidden as shown below.

{{< figure src="../images/post_labeled_image.en.png" width="60%" >}}


