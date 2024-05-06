import QtQuick 2.15
import tech.relog.hagoromo.accountlistmodel 1.0

QtObject {
    id: obj
    property string uuid: ""
    property string service: ""
    property string did: ""
    property string handle: ""
    property string email: ""
    property string accessJwt: ""
    property string refreshJwt: ""
    property string avatar: ""
    property string serviceEndpoint: ""

    function set(model, uuid){
        var row = model.indexAt(uuid)
        if(row >= 0){
            obj.uuid = uuid
            obj.service = model.item(row, AccountListModel.ServiceRole)
            obj.did = model.item(row, AccountListModel.DidRole)
            obj.handle = model.item(row, AccountListModel.HandleRole)
            obj.email = model.item(row, AccountListModel.EmailRole)
            obj.accessJwt = model.item(row, AccountListModel.AccessJwtRole)
            obj.refreshJwt = model.item(row, AccountListModel.RefreshJwtRole)
            obj.avatar = model.item(row, AccountListModel.AvatarRole)
            obj.serviceEndpoint = model.item(row, AccountListModel.ServiceEndpointRole)

            return true
        }else{
            return false
        }
    }
}
