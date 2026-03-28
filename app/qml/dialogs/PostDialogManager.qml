import QtQuick 2.15

Item {
    id: postDialogManager

    property int parentWidth: 0
    property int parentHeight: 0
    property var accountModel: null
    property bool working: repeater.working

    signal errorOccurred(string account_uuid, string code, string message)
    signal requestNotifyProgress(string itemId, string contentId, string headerText, string message, bool fixedWidth)
    signal requestClearProgress(string itemId, string contentId)
    signal requestClearAllProgress(string itemId)

    function open(post_type, account_uuid, cid, uri, reply_root_cid, reply_root_uri,
                  avatar, display_name, handle, indexed_at, text, image_urls) {
        repeater.open(post_type, account_uuid, cid, uri, reply_root_cid, reply_root_uri,
                      avatar, display_name, handle, indexed_at, text, image_urls)
    }

    Component {
        id: postDialogComponent
        PostDialog {
            id: postDialog
            parentWidth: postDialogManager.parentWidth
            parentHeight: postDialogManager.parentHeight
            accountModel: postDialogManager.accountModel
            onErrorOccurred: (account_uuid, code, message) => postDialogManager.errorOccurred(account_uuid, code, message)
            onClosed: repeater.remove(dialog_no)
            onChangeActiveDialog: (dialog_no, active) => {
                if(active) {
                    repeater.updateActiveDialog(dialog_no)
                }
            }
        }
    }
    Repeater {
        id: repeater
        property int dialog_no: 0
        property var depth_list: []
        property bool working: false
        model: ListModel {}
        onWorkingChanged: console.log("!!!!!!! working = " + working + "  !!!!!!!!!")
        Loader {
            required property int index
            required property int dialog_no
            required property int dialog_default_x
            required property int dialog_default_y
            required property string post_type
            required property string account_uuid
            required property string cid
            required property string uri
            required property string reply_root_cid
            required property string reply_root_uri
            required property string avatar
            required property string display_name
            required property string handle
            required property string indexed_at
            required property string text
            required property string image_urls

            sourceComponent: postDialogComponent
            onLoaded: {
                item.requestNotifyProgress.connect(postDialogManager.requestNotifyProgress)
                item.requestClearProgress.connect(postDialogManager.requestClearProgress)
                item.requestClearAllProgress.connect(postDialogManager.requestClearAllProgress)

                item.dialog_no = dialog_no
                item.dialog_default_x = dialog_default_x
                item.dialog_default_y = dialog_default_y
                item.postType = post_type
                item.defaultAccountUuid = account_uuid
                if(item.postType === "reply"){
                    item.replyCid = cid
                    item.replyUri = uri
                    item.replyRootCid = reply_root_cid
                    item.replyRootUri = reply_root_uri
                    item.replyAvatar = avatar
                    item.replyDisplayName = display_name
                    item.replyHandle = handle
                    item.replyIndexedAt = indexed_at
                    item.replyText = text
                }else if(item.postType === "quote"){
                    item.quoteCid = cid
                    item.quoteUri = uri
                    item.quoteAvatar = avatar
                    item.quoteDisplayName = display_name
                    item.quoteHandle = handle
                    item.quoteIndexedAt = indexed_at
                    item.quoteText = text
                }else if(handle.length > 0){
                    item.postText.text = handle + " "
                }
                if(image_urls.length === 0){
                    item.open()
                }else{
                    item.openWithFiles(image_urls.split("\n"))
                }
                repeater.working = false
                console.timeEnd("post_dialog_open");
            }
        }
        function open(post_type, account_uuid, cid, uri, reply_root_cid, reply_root_uri,
                      avatar, display_name, handle, indexed_at, text, image_urls) {
            console.time("post_dialog_open");
            var top_pos = repeater.getTopPosition()
            working = true
            repeater.depth_list.push(repeater.dialog_no)
            repeater.model.append({
                                       "dialog_no": repeater.dialog_no++,
                                       "post_type": post_type,
                                       "dialog_default_x": top_pos[0],
                                       "dialog_default_y": top_pos[1],
                                       "account_uuid": account_uuid,
                                       "cid": cid,
                                       "uri": uri,
                                       "reply_root_cid": reply_root_cid,
                                       "reply_root_uri": reply_root_uri,
                                       "avatar": avatar,
                                       "display_name": display_name,
                                       "handle": handle,
                                       "indexed_at": indexed_at,
                                       "text": text,
                                       "image_urls": image_urls.join("\n")
                                   })
        }
        function remove(no){
            var living_dialog = []
            var remove_index = -1
            for(var i=0;i<count;i++){
                var loader_item = itemAt(i)
                console.log("remove : no=" + no + ", Item no=" + loader_item.item.dialog_no)
                if(no === loader_item.item.dialog_no){
                    remove_index = i
                }else{
                    living_dialog.push(loader_item.item.dialog_no)
                }
            }
            if(remove_index >= 0){
                repeater.model.remove(remove_index)
            }
            // 存在しているダイアログのみにする
            repeater.depth_list = repeater.depth_list.filter((t) => living_dialog.indexOf(t) >= 0)
            console.log(repeater.depth_list)
        }
        function updateActiveDialog(no){
            var new_depth_list = repeater.depth_list
            var current_index = repeater.depth_list.indexOf(no)
            if(current_index < 0 || current_index >= (repeater.depth_list.length - 1)){
                return
            }
            repeater.depth_list.splice(current_index, 1)
            repeater.depth_list.push(no)

            // いったん0にしないと意図どおり反映されない
            for(var i=0;i<count;i++){
                var loader_item = itemAt(i)
                loader_item.item.dialog_z = 0
            }
            // zオーダーを再設定しつつ、管理リストから既に消えているdialog_noを消す
            var living_dialog = []
            repeater.depth_list.forEach((list_no, list_index) => {
                                            for(var i=0;i<count;i++){
                                                var loader_item = itemAt(i)
                                                if(loader_item.item.dialog_no === list_no){
                                                    living_dialog.push(list_no)
                                                    loader_item.item.dialog_z = list_index + 1
                                                }
                                            }
                                        })
            // 存在しているダイアログのみにする
            repeater.depth_list = repeater.depth_list.filter((no) => living_dialog.indexOf(no) >= 0)
        }
        function getTopPosition(){
            var l = repeater.depth_list.length
            if(l === 0){
                return [-1, -1]
            }
            var top_dialog_no = repeater.depth_list[l-1]
            for(var i=0;i<count;i++){
                var loader_item = itemAt(i)
                if(top_dialog_no === loader_item.item.dialog_no){
                    return [loader_item.item.dialog_x, loader_item.item.dialog_y]
                }
            }
            return [-1, -1]
        }
    }
}
