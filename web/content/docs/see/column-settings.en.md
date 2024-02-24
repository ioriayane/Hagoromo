---
title: Column settings
weight: 90
---

# Column settings

{{< figure src="../images/column-settings.en.png" width="100%" >}}

## Auto loading

The column contents are automatically reloaded every time selected in the `Interval`.

## Interval

The interval between reloads.

## Column width

Width can be adjusted from 300 to 500.

50 increments.

## Image layout

The layout of the preview image to be displayed when an image is attached.

- `Compact` : Displays up to 4 images on a single line.
- `Normal` : Displays the image in a 2x2 layout.
- `When one is whole` : If there are more than 2 images, the same 2x2 layout as `Normal` will be displayed.
- `All whole` : All four images are displayed vertically in a row.

## Display

You can select what to display in a column.

Since the data is retained, it can be restored once hidden.

### Notification column

You can choose to show or hide the following

- `Like`
- `Repost`
- `Follow`
- `Mention`
- `Reply`
- `Quote`

### Home (Following)

You can choose to display or not display the following

- `Reply to unfollowed users`
  - If off, only replies to users you are following will be displayed.
- Repost
  - When set to off, it will be hidden as shown in the table below.

|Reposted by→|You|You|You|Following|Following(A)|Following(A)|Following|
|---|---|---|---|---|---|---|---|
|Was reposted→|You|Following|Not following|You|Following(A)|Following(B)|Not following|
|Reposts by the user|✔|-|-|-|✔|-|-|
|Reposts of users followed|-|✔|-|-|✔|✔|-|
|Reposts of users you do not follow|-|-|✔|-|-|-|✔|
|Reposts of your posts|✔|-|-|✔|-|-|-|
|Reposts by you|✔|✔|✔|-|-|-|-|

|Item|Supplemental explanation of selection criteria|
|---|---|
|Reposts by the user|Who is reposting who?|
|Reposts of users followed|Who you are reposting|
|Reposts of users you do not follow|Who you are reposting|
|Reposts of your posts|Who you are reposting|
|Reposts by you|Who is reposting|

## Combination of column type and display content

|Item|Home|Notification|Search Post|Search Profile|Custom feed|User feed|List|
|---|---|---|---|---|---|---|---|
|Auto loading|✔|✔|✔|✔|✔|✔|✔|
|Interval|✔|✔|✔|✔|✔|✔|✔|
|Column width|✔|✔|✔|✔|✔|✔|✔|
|Image layout|✔|✔|✔||✔|✔|✔|
|Display - Reply to unfollowed users|✔|||||||
|Display - Reposts by the user|✔|||||||
|Display - Reposts of users followed|✔|||||||
|Display - Reposts of users you do not follow|✔|||||||
|Display - Reposts of your posts|✔|||||||
|Display - Reposts by you|✔|||||||
|Display - Like||✔|||||
|Display - Repost||✔|||||
|Display - Follow||✔|||||
|Display - Mention||✔|||||
|Display - Reply||✔|||||
|Display - Quote||✔|||||
