---
title: Realtime feed
weight: 30
---

# Realtime feed

This is a function that uses [Jetstream](https://docs.bsky.app/blog/jetstream) to display posts as they are posted.

Unlike the usual timeline, where posts are collected together at regular intervals, you can enjoy a more realistic feed.

## How to add to the column

Select from the column type `Realtime Feeds` in the dialog box for adding columns.

{{< figure src="../images/realtime-select-feed.en.png" width="80%" >}}

You can choose from the following preset feeds, or feeds that you have created yourself.

- `Following`
  - The same content as the official app's `Following`
- `Followers`
  - Posts from accounts that follow you (excluding you)
- `Mutual following`
  - Posts from accounts that are following each other (including you)
- `Following and Followers`
  - Posts from both the accounts you are following and the accounts that are following you (including you)

## About the columns in the Realtime feed

The icon shown in the red circle in the realtime feed column indicates the status of the post.

{{< figure src="../images/realtime-column-status.en.png" width="80%" >}}

- `Green` : Receiving normally
- `Gray` : Receiving stopped due to some kind of trouble

When it is grayed out, pressing the button indicated by the red square will attempt to restore it.
If it can be restored, it will turn green.

## Creating a Realtime feed

You can create your own realtime feed.

If you select `Create Realtime Feed` in the dialog for adding columns, the editing dialog will be displayed.

{{< figure src="../images/realtime-editing-1.en.png" width="80%" >}}

- `Name`
  - Required
  - Cannot be changed、Once you have created it, if you change it and save it, it will be treated as a new feed
- `Condition`
  - Required
  - Use the `New` button to build your conditions

### Condition Construction

The conditions are constructed by selecting from two types of items: operation items and account items.

{{< figure src="../images/realtime-add-condition-item.en.png" width="50%" >}}

- Operation items
  - `AND`
  - `OR`
  - `XOR`
- Account items
  - `Following`
  - `Followers`
  - `List`
  - `Me`

You can delete items from the menu that appears when you right-click. 

{{< figure src="../images/realtime-delete-condition-item.en.png" width="40%" >}}

#### About operation items

You can add both operation items and account items to the operation item.

Example：
{{< figure src="../images/realtime-condition.en.png" width="50%" >}}

#### About Account Items

The following additional conditions can be set as detailed settings for account items.

{{< figure src="../images/realtime-item-detail.en.png" width="80%" >}}

- List
  - Only available for lists
  - Required
- Image
  - Selection items
    - `0` : Posts without images attached
    - `1`～`4` : Post with only the selected number of images attached
    - `>=1` : Posts with images attached
  - Videos are exclusive.
  - If you set this, reposts will be excluded from the feed.
- Video
  - Selection items
    - `0` : Posts without Videos attached
    - `1` : Posts with Videos attached
  - Images are exclusive
  - If you set this, reposts will be excluded from the feed.
- Quote
  - Selection items
    - `Only` : Only posts that include quotations
    - `Exclude` : Exclude posts that contain quotes
  - If you select `Only`, reposts will be excluded from the feed
- Repost
  - Selection items
    - `Only` : Only reposts
    - `Exclude` : Exclude reposts

## Editing and deleting Realtime feeds

To edit or delete the realtime feed you have created, use the menu that appears when you right-click.

{{< figure src="../images/realtime-edit-delete.en.png" width="80%" >}}
