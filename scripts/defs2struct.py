# レキシコンのjsonから構造体の定義するヘッダーを出力するツール
# https://github.com/bluesky-social/atproto/tree/main/lexicons
# query系のoutputからの参照を起点に定義を作った方が良いかもしれない

# python3 ./scripts/defs2struct.py

import os
import glob
import json
from jinja2 import Environment, FileSystemLoader

class FuncHistoryItem:
    """ 関数の履歴保存用 """
    def __init__(self, type_name, qt_type) -> None:
        self.type_name = type_name
        self.qt_type = qt_type

class FunctionArgument:
    """ 関数の引数管理用 """
    def __init__(self, arg_type: str, name: str, is_array: bool) -> None:
        self._type = arg_type
        self._name = name
        self._is_array = is_array

    def to_string_arg(self) -> str:
        """ コードの文字列に変換 """
        arg_def: str = ''
        if self._is_array:
            if self._type == 'string':
                arg_def = f"const QList<QString> &{self._name}"
            elif self._type == 'integer':
                arg_def =  f"const QList<int> &{self._name}"
            elif self._type == 'boolean':
                arg_def = f"const QList<bool> &{self._name}"
        elif self._type == 'string' or self._type == 'json_string':
            arg_def = f"const QString &{self._name}"
        elif self._type == 'integer':
            arg_def = f"const int {self._name}"
        elif self._type == 'boolean':
            arg_def = f"const bool {self._name}"
        elif self._type == 'unknown':
            arg_def = f"const QJsonObject &{self._name}"
        elif self._type == 'json_array':
            arg_def = f"const QJsonArray &{self._name}"
        elif self._type == 'object':
            arg_def = f"const QJsonObject &{self._name}"
        elif self._type == 'json_object':
            arg_def = f"const QJsonObject &{self._name}"
        return arg_def

    def to_string_query(self) -> str:
        """ クエリの作成 """
        query: str = ''
        if self._is_array:
            query  = f"for (const auto &value : {self._name})" + "{\n"
            query += f"url_query.addQueryItem(QStringLiteral(\"{self._name}\"), value);\n"
            query += "}\n"
        elif self._type == 'string':
            query  = f"if(!{self._name}.isEmpty())" + "{\n"
            query += f"url_query.addQueryItem(QStringLiteral(\"{self._name}\"), {self._name});\n"
            query += "}\n"
        elif self._type == 'integer':
            query  = f"if({self._name} > 0)" + "{\n"
            query += f"url_query.addQueryItem(QStringLiteral(\"{self._name}\"), QString::number({self._name}));\n"
            query += "}\n"
        elif self._type == 'boolean':
            query  = f"if({self._name})" + "{\n"
            query += f"url_query.addQueryItem(QStringLiteral(\"{self._name}\"), \"true\");\n"
            query += "}\n"
        return query

    def to_string_payload(self) -> str:
        """ postのデータ """
        payload: str = ''
        if self._is_array:
            payload  = f"for (const auto &value : {self._name})" + "{\n"
            payload += f"url_query.addQueryItem(QStringLiteral(\"{self._name}\"), value);\n"
            payload += "}\n"
        elif self._type == 'string' or self._type == 'json_string':
            payload  = f"if(!{self._name}.isEmpty())" + "{\n"
            payload += f"json_obj.insert(QStringLiteral(\"{self._name}\"), {self._name});\n"
            payload += "}\n"
        elif self._type == 'integer':
            payload  = f"if({self._name} > 0)" + "{\n"
            payload += f"json_obj.insert(QStringLiteral(\"{self._name}\"), QString::number({self._name}));\n"
            payload += "}\n"
        elif self._type == 'boolean':
            # payload  = f"if({self._name})" + "{\n"
            payload += f"json_obj.insert(QStringLiteral(\"{self._name}\"), {self._name});\n"
            # payload += "}\n"
        elif self._type == 'unknown' or self._type == 'json_array' or self._type == 'object' or self._type == 'json_object':
            payload  = f"if(!{self._name}.isEmpty())" + "{\n"
            payload += f"json_obj.insert(QStringLiteral(\"{self._name}\"), {self._name});\n"
            payload += "}\n"
        return payload


class Defs2Struct:
    """ lexiconの定義から構造体などを生成 """
    def __init__(self) -> None:
        self.history = [] # <namespace>#<struct_name>
        self.history_namespace = []
        self.history_func = {}
        self.history_pointer = []   # namespace#type#property#ref_namespace#ref_type
        self.json_obj = {}  # [namespace] = obj
        self.output_text = {} # [namespace] = list(line)
        self.output_func_text = {} # [namespace] = list(line)
        self.namespace_stack = []
        self.pre_define = {} # [namespace] = <struct_name>
        self.history_type = {}  #[namespace#struct_name] = type
        self.converted_extend_paths = []    # 処理済みの拡張lexiconのパス

        self.api_class = {}

        # QVariantに入れる構造体をQ_DECLARE_METATYPE()で定義する構造体
        # （単純にすべてではない）
        self.metatype = ('AppBskyFeedPost::Main',
                         'AppBskyFeedLike::Main',
                         'AppBskyFeedRepost::Main',
                         'AppBskyGraphListitem::Main',
                         'AppBskyActorProfile::Main',
                         'AppBskyGraphList::Main',
                         'AppBskyFeedThreadgate::Main',
                        )
        self.inheritance = {
                'app.bsky.actor.defs#profileView': {
                    'parent_namespace': 'app.bsky.graph.getFollows',
                    'parent_header': ['atprotocol/app/bsky/graph/appbskygraphgetfollows.h']
                },
                'app.bsky.actor.defs#profileViewBasic': {
                    'parent_namespace': 'app.bsky.graph.getFollows',
                    'parent_header': ['atprotocol/app/bsky/graph/appbskygraphgetfollows.h']
                },
                'app.bsky.feed.defs#generatorView': {
                    'parent_namespace': 'app.bsky.feed.getFeedGenerators',
                    'parent_header': ['atprotocol/app/bsky/feed/appbskyfeedgetfeedgenerators.h']
                },
                'app.bsky.feed.defs#feedViewPost': {
                    'parent_namespace': 'app.bsky.feed.getTimeline',
                    'parent_header': ['atprotocol/app/bsky/feed/appbskyfeedgettimeline.h']
                },
                'app.bsky.feed.defs#postView': {
                    'parent_namespace': 'app.bsky.feed.getPosts',
                    'parent_header': ['atprotocol/app/bsky/feed/appbskyfeedgetposts.h']
                },
                'app.bsky.graph.defs#listView': {
                    'parent_namespace': 'app.bsky.graph.getLists',
                    'parent_header': ['atprotocol/app/bsky/graph/appbskygraphgetlists.h']
                }
            }

        self.rawHttpHeader = {
            'chat.bsky.': {
                'name': 'atproto-proxy',
                'value': 'did:web:api.bsky.chat#bsky_chat'
            }
        }

        self.skip_api_class_id = [
            'tools.ozone.',
            'com.atproto.admin.',
            'com.atproto.identity.',
            'com.atproto.label.',
            'com.atproto.repo.applyWrites',
            'com.atproto.repo.describeRepo',
            'com.atproto.repo.importRepo',
            'com.atproto.repo.uploadBlob',
            'com.atproto.repo.listMissingBlobs',
            'com.atproto.server.activateAccount',
            'com.atproto.server.checkAccountStatus',
            'com.atproto.server.confirmEmail',
            'com.atproto.server.createAccount',
            'com.atproto.server.createAppPassword',
            'com.atproto.server.createInviteCode',
            'com.atproto.server.createInviteCodes',
            'com.atproto.server.deactivateAccount',
            'com.atproto.server.deleteAccount',
            'com.atproto.server.deleteSession',
            'com.atproto.server.describeServer',
            'com.atproto.server.getAccountInviteCodes',
            'com.atproto.server.getServiceAuth',
            'com.atproto.server.getSession',
            'com.atproto.server.listAppPasswords',
            'com.atproto.server.requestAccountDelete',
            'com.atproto.server.requestEmailConfirmation',
            'com.atproto.server.requestEmailUpdate',
            'com.atproto.server.requestPasswordReset',
            'com.atproto.server.reserveSigningKey',
            'com.atproto.server.resetPassword',
            'com.atproto.server.revokeAppPassword',
            'com.atproto.server.updateEmail',
            'com.atproto.sync.getHead',
            'com.atproto.sync.getLatestCommit',
            'com.atproto.sync.getBlocks',
            'com.atproto.sync.getCheckout',
            'com.atproto.sync.getRecord',
            'com.atproto.sync.getRepo',
            'com.atproto.sync.notifyOfUpdate',
            'com.atproto.sync.requestCrawl',
            'com.atproto.sync.listBlobs',
            'com.atproto.sync.listRepos',
            'com.atproto.temp.',
            'app.bsky.unspecced.searchPostsSkeleton',
            'app.bsky.unspecced.searchActorsSkeleton',
            'app.bsky.unspecced.getSuggestionsSkeleton',
            'app.bsky.unspecced.getTaggedSuggestions',
            'app.bsky.notification.registerPush',
            'app.bsky.notification.getUnreadCount',
            'app.bsky.graph.getSuggestedFollowsByActor',
            'app.bsky.graph.getRelationships',
            'app.bsky.feed.sendInteractions',
            'app.bsky.feed.getSuggestedFeeds',
            'app.bsky.feed.getFeedSkeleton',
            'app.bsky.feed.describeFeedGenerator',
            'app.bsky.actor.getSuggestions'
        ]
        self.unuse_auth = [
            'com.atproto.server.createSession',
            'com.atproto.sync.getBlob',
        ]
        self.need_extension = [
            'com.atproto.moderation.createReport',
            'com.atproto.repo.createRecord',
            'com.atproto.repo.deleteRecord',
            'com.atproto.repo.getRecord',
            'com.atproto.repo.listRecords',
            'com.atproto.repo.putRecord',
            'com.atproto.server.createSession',
            'com.atproto.server.refreshSession'
        ]

    def skip_spi_class(self, namespace: str) -> bool:
        for class_id in self.skip_api_class_id:
            if namespace.startswith(class_id):
                return True
        return False

    def to_struct_style(self, name: str) -> str:
        return name[0].upper() + name[1:]

    def to_namespace_style(self, name: str) -> str:
        srcs = name.split('.')
        dest = []
        for src in srcs:
            dest.append(src[0].upper() + src[1:])
            # app.bsky.embed.recordWithMediaがあるのでcapitalize()は使えない
        return ''.join(dest)

    def to_header_path(self, namespace: str) -> str:
        srcs = namespace.split('.')
        dest = ['atprotocol']
        dest.extend(srcs[:-1])
        dest.append(namespace.lower().replace('.', '') + '.h')
        return '/'.join(dest)

    def split_ref(self, path: str) -> tuple:
        if '#' in path:
            return path.split('#')
        else:
            return (path, '')

    def get_defs_obj(self, namespace: str, type_name: str) -> dict:
        return self.json_obj.get(namespace, {}).get('defs', {}).get(type_name, {})

    def append_history(self, namespace: str, type_name: str):
        self.history.append(namespace + '#' + type_name)
        if namespace not in self.history_namespace:
            self.history_namespace.append(namespace)

    def append_func_history(self, namespace: str, function_define: str):
        if namespace not in self.history_func:
            self.history_func[namespace] = [function_define]
        else:
            self.history_func[namespace].append(function_define)


    def append_pre_define(self, namespace: str, type_name: str):
        key = self.to_namespace_style(namespace)
        value = self.to_struct_style(type_name)
        if key in self.pre_define:
            if value not in self.pre_define[key]:
                self.pre_define[key].append(value)
        else:
            self.pre_define[key] = [value]

    def index_of(self, target_list: list, v: str) -> int:
        return target_list.index(v) if v in target_list else len(target_list)

    def defined_before(self, other: str, me: str) -> bool:
        if other in self.history_namespace:
            other_pos = self.history_namespace.index(other)
        else:
            other_pos = -1
        if me in self.history_namespace:
            me_pos = self.history_namespace.index(me)
        else:
            me_pos = -1
        if other_pos == -1:
            return False
        elif me_pos == -1:
            return True
        else:
            return other_pos < me_pos

    def make_comment_string(self, properties: dict) -> str:
        formatting = properties.get('format', '').strip()
        description = properties.get('description', '').strip()
        comment = ''
        if len(formatting) > 0:
            comment += ' // ' + formatting
        if len(description) > 0:
            if len(comment) > 0:
                comment += ' , '
            else:
                comment += ' // '
            comment += description
        return comment

    def check_deprecated(self, properties: dict) -> bool:
        return ('deprecated' in properties.get('description', '').strip().lower())

    def output_ref(self, namespace: str, type_name: str, property_name: str, ref_obj: dict, comment: str, is_array: bool = False):
        (ref_namespace, ref_struct_name) = self.split_ref(ref_obj)
        if len(ref_struct_name) == 0:
            ref_struct_name = 'main'

        if len(ref_namespace) == 0:
            if not is_array:
                if (namespace + '#' + ref_struct_name) in self.namespace_stack:
                    self.append_pre_define(namespace, ref_struct_name)
                    self.history_pointer.append(namespace + '#' + type_name + '#' + property_name + '#' + namespace + '#' + ref_struct_name)
                    self.output_text[namespace].append('    QSharedPointer<%s> %s;%s' % (
                        self.to_struct_style(ref_struct_name), property_name, comment,
                        ))
                else:
                    self.output_text[namespace].append('    %s %s;%s' % (
                        self.to_struct_style(ref_struct_name), property_name, comment,
                        ))
            else:
                if (namespace + '#' + ref_struct_name) in self.namespace_stack:
                    self.append_pre_define(namespace, ref_struct_name)
                    self.history_pointer.append(namespace + '#' + type_name + '#' + property_name + '#' + namespace + '#' + ref_struct_name)
                    self.output_text[namespace].append('    QList<QSharedPointer<%s>> %s;%s' % (
                        self.to_struct_style(ref_struct_name), property_name, comment,
                        ))
                else:
                    self.output_text[namespace].append('    QList<%s> %s;%s' % (
                        self.to_struct_style(ref_struct_name), property_name, comment,
                        ))
        else:
            # 履歴で自分の方が前にいるということは宣言が後ろなのでポインタにする
            # 自分が履歴にいないときは宣言が前
            my_history_pos = self.index_of(self.history_namespace, namespace)
            ref_history_pos = self.index_of(self.history_namespace, ref_namespace)
            if not is_array:
                if (ref_namespace + '#' + ref_struct_name) in self.namespace_stack or (my_history_pos < ref_history_pos):
                    self.append_pre_define(ref_namespace, ref_struct_name)
                    self.history_pointer.append(namespace + '#' + type_name + '#' + property_name + '#' + ref_namespace + '#' + ref_struct_name)
                    self.output_text[namespace].append('    QSharedPointer<%s::%s> %s;' % (
                        self.to_namespace_style(ref_namespace), self.to_struct_style(ref_struct_name), property_name,
                        ))
                else:
                    self.output_text[namespace].append('    %s::%s %s;' % (
                        self.to_namespace_style(ref_namespace), self.to_struct_style(ref_struct_name), property_name,
                        ))
            else:
                if (ref_namespace + '#' + ref_struct_name) in self.namespace_stack or (my_history_pos < ref_history_pos):
                    self.append_pre_define(ref_namespace, ref_struct_name)
                    self.history_pointer.append(namespace + '#' + type_name + '#' + property_name + '#' + ref_namespace + '#' + ref_struct_name)
                    self.output_text[namespace].append('    QList<QSharedPointer<%s::%s>> %s;' % (
                        self.to_namespace_style(ref_namespace), self.to_struct_style(ref_struct_name), property_name
                        ))
                else:
                    self.output_text[namespace].append('    QList<%s::%s> %s;' % (
                        self.to_namespace_style(ref_namespace), self.to_struct_style(ref_struct_name), property_name
                        ))


    def output_union(self, namespace: str, type_name: str, property_name: str, refs_obj: dict, comment: str, is_array: bool = False):
        pointer_list = []
        list_pointer_list = []
        enum_text = []
        union_name_list = []
        self.output_text[namespace].append('    // union start : %s' % (property_name, ))
        pos = len(self.output_text[namespace])
        for ref in refs_obj:
            (ref_namespace, ref_struct_name) = self.split_ref(ref)
            if len(ref_struct_name) == 0:
                ref_struct_name = 'main'

            if len(ref_namespace) == 0:
                union_name = '%s_%s' % (property_name, self.to_struct_style(ref_struct_name))
                union_name_list.append(union_name)
                if (namespace + '#' + ref_struct_name) in self.namespace_stack:
                    if not is_array:
                        pointer_list.append(union_name)
                    else:
                        list_pointer_list.append(union_name)
                    self.append_pre_define(namespace, ref_struct_name)
                    self.history_pointer.append(namespace + '#' + type_name + '#' + property_name + '#' + namespace + '#' + ref_struct_name)
                    if not is_array:
                        self.output_text[namespace].append('    QSharedPointer<%s> %s;%s' % (
                            self.to_struct_style(ref_struct_name), union_name, comment,
                            ))
                    else:
                        self.output_text[namespace].append('    QList<QSharedPointer<%s>> %s;%s' % (
                            self.to_struct_style(ref_struct_name), union_name, comment,
                            ))
                else:
                    if not is_array:
                        self.output_text[namespace].append('    %s %s;%s' % (
                            self.to_struct_style(ref_struct_name), union_name, comment,
                            ))
                    else:
                        self.output_text[namespace].append('    QList<%s> %s;%s' % (
                            self.to_struct_style(ref_struct_name), union_name, comment,
                            ))
            else:
                union_name = '%s_%s_%s' % (property_name, self.to_namespace_style(ref_namespace), self.to_struct_style(ref_struct_name))
                union_name_list.append(union_name)
                if not is_array:
                    if (ref_namespace + '#' + ref_struct_name) in self.namespace_stack or not self.defined_before(ref_namespace, namespace):
                        # 参照先の名前空間が履歴的に自分より後に出力することになっている場合もこちら
                        # 他の箇所も同様の処置をするべきな気がするけど、現状ここだけでOK
                        pointer_list.append(union_name)
                        self.append_pre_define(ref_namespace, ref_struct_name)
                        self.history_pointer.append(namespace + '#' + type_name + '#' + property_name + '#' + ref_namespace + '#' + ref_struct_name)
                        self.output_text[namespace].append('    QSharedPointer<%s::%s> %s;%s' % (
                            self.to_namespace_style(ref_namespace), self.to_struct_style(ref_struct_name), union_name, comment,
                            ))
                    else:
                        self.output_text[namespace].append('    %s::%s %s;%s' % (
                            self.to_namespace_style(ref_namespace), self.to_struct_style(ref_struct_name), union_name, comment,
                            ))
                else:
                    if (ref_namespace + '#' + ref_struct_name) in self.namespace_stack:
                        pointer_list.append(union_name)
                        self.append_pre_define(ref_namespace, ref_struct_name)
                        self.history_pointer.append(namespace + '#' + type_name + '#' + property_name + '#' + ref_namespace + '#' + ref_struct_name)
                        self.output_text[namespace].append('    QList<QSharedPointer<%s::%s>> %s;%s' % (
                            self.to_namespace_style(ref_namespace), self.to_struct_style(ref_struct_name), union_name, comment,
                            ))
                    else:
                        self.output_text[namespace].append('    QList<%s::%s> %s;%s' % (
                            self.to_namespace_style(ref_namespace), self.to_struct_style(ref_struct_name), union_name, comment,
                            ))
        self.output_text[namespace].append('    // union end : %s' % (property_name, ))

        # enumの定義挿入
        if len(union_name_list) > 0:
            union_type_name = '%s%sType' % (self.to_struct_style(type_name), self.to_struct_style(property_name), )
            self.output_text[namespace].insert(pos, '    %s %s_type = %s::none;' % (union_type_name, property_name, union_type_name, ))
            enum_text.append('enum class %s : int {' % (union_type_name, ))
            enum_text.append('    none,')
            for union_name in union_name_list:
                enum_text.append('    %s,' % (union_name, ))
            enum_text.append('};')

        return (pointer_list, list_pointer_list, enum_text)

    def output_ref_recursive(self, namespace: str, type_name: str, ref: str):
        (ref_namespace, ref_struct_name) = self.split_ref(ref)
        if len(ref_struct_name) == 0:
            ref_struct_name = 'main'

        if len(ref_namespace) == 0:
            # if ref_struct_name == type_name:
            if (namespace + '#' + ref_struct_name) in self.namespace_stack:
                # infinite loop
                pass
            else:
                self.output_type(namespace, ref_struct_name, self.get_defs_obj(namespace, ref_struct_name))
        else:
            if (ref_namespace + '#' + ref_struct_name) in self.namespace_stack:
            # if ref_namespace == namespace and ref_struct_name == type_name:
                # infinite loop
                pass
            else:
                if ref_namespace != namespace and ref_namespace not in self.history_namespace:
                    # 名前空間が違う場合は自分の名前空間より前に履歴を差し込む
                    # 同一名前空間で参照のない構造体を先に処理すると順番が後になってしまうため
                    for i, value in enumerate(self.history_namespace):
                        if value == namespace:
                            self.history_namespace.insert(i, ref_namespace)
                            break
                self.output_type(ref_namespace, ref_struct_name, self.get_defs_obj(ref_namespace, ref_struct_name))

    def output_type(self, namespace: str, type_name: str, obj: dict):
        if (namespace + '#' + type_name) in self.history:
            # already outputted
            return

        self.namespace_stack.append(namespace + '#' + type_name)

        if namespace not in self.output_text:
            self.output_text[namespace] = []

        self.history_type[namespace + '#' + type_name] = obj.get('type')
        obj_comment = self.make_comment_string(obj)

        if obj.get('type') == 'object':
            # 構造体
            properties = obj.get('properties', {})
            # refかunionのときはその型を先に処理する
            for property_name in properties.keys():
                p_type = properties[property_name].get('type')
                if p_type == 'ref':
                    self.output_ref_recursive(namespace, type_name, properties[property_name].get('ref', ''))

                elif p_type == 'union':
                    for ref in properties[property_name].get('refs', []):
                        self.output_ref_recursive(namespace, type_name, ref)

                elif p_type == 'array':
                    items_type = properties[property_name].get('items', {}).get('type', '')
                    if items_type == 'ref':
                        self.output_ref_recursive(namespace, type_name, properties[property_name].get('items', {}).get('ref', ''))
                    elif items_type == 'union':
                        for ref in properties[property_name].get('items', {}).get('refs', []):
                            self.output_ref_recursive(namespace, type_name, ref)

            # 実際に出力する
            enum_text = []
            styled_type_name = self.to_struct_style(type_name)
            self.output_text[namespace].append('struct %s' % (styled_type_name, ))
            self.output_text[namespace].append('{')
            for property_name in properties.keys():
                if self.check_deprecated(properties[property_name]):
                    continue
                p_type = properties[property_name].get('type')
                p_comment = self.make_comment_string(properties[property_name])
                if p_type == 'ref':
                    self.output_ref(namespace, type_name, property_name, properties[property_name].get('ref', {}), p_comment)
                elif p_type == 'union':
                    (temp_pointer, temp_list_pointer, temp_enum) = self.output_union(namespace, type_name, property_name, properties[property_name].get('refs', []), p_comment)
                    enum_text.extend(temp_enum)
                elif p_type == 'unknown':
                    self.output_text[namespace].append('    QVariant %s;%s' % (property_name, p_comment, ))
                elif p_type == 'integer':
                    self.output_text[namespace].append('    int %s = 0;%s' % (property_name, p_comment, ))
                elif p_type == 'boolean':
                    self.output_text[namespace].append('    bool %s = false;%s' % (property_name, p_comment, ))
                elif p_type == 'string':
                    self.output_text[namespace].append('    QString %s;%s' % (property_name, p_comment, ))
                elif p_type == 'array':
                    items_type = properties[property_name].get('items', {}).get('type', '')
                    if items_type == 'ref':
                        self.output_ref(namespace, type_name, property_name, properties[property_name].get('items', {}).get('ref', {}), p_comment, True)
                    elif items_type == 'union':
                        (temp_pointer, temp_list_pointer, temp_enum) = self.output_union(namespace, type_name, property_name, properties[property_name].get('items', {}).get('refs', []), p_comment, True)
                        enum_text.extend(temp_enum)
                    elif items_type == 'integer':
                        self.output_text[namespace].append('    QList<int> %s;%s' % (property_name, p_comment, ))
                    elif items_type == 'boolean':
                        self.output_text[namespace].append('    QList<bool> %s;%s' % (property_name, p_comment, ))
                    elif items_type == 'string':
                        self.output_text[namespace].append('    QList<QString> %s;%s' % (property_name, p_comment, ))
                elif p_type == 'blob':
                    self.output_text[namespace].append('    Blob %s;%s' % (property_name, p_comment, ))

            self.output_text[namespace].append('};')

            # enumの定義を名前空間の先頭に挿入
            if len(enum_text) > 0:
                ins_pos = 0
                for enum_line in enum_text:
                    self.output_text[namespace].insert(ins_pos, enum_line)
                    ins_pos += 1


            self.append_history(namespace, type_name)

        elif obj.get('type') == 'string':
            # 文字列は型定義にする
            self.output_text[namespace].append('typedef QString %s;%s' % (self.to_struct_style(type_name), obj_comment,))
            self.append_history(namespace, type_name)

        elif obj.get('type') == 'integer':
            # 数値は型定義にする
            self.output_text[namespace].append('typedef int %s;%s' % (self.to_struct_style(type_name), obj_comment,))
            self.append_history(namespace, type_name)

        elif obj.get('type') == 'boolean':
            # boolは型定義にする
            self.output_text[namespace].append('typedef bool %s;%s' % (self.to_struct_style(type_name), obj_comment,))
            self.append_history(namespace, type_name)

        elif obj.get('type') == 'array':
            # arrayは型定義にする
            items_type = obj.get('items', {}).get('type', '')
            if items_type == 'union':
                # refのときはその型を先に処理する
                for ref_path in obj.get('items', {}).get('refs', []):
                    self.output_ref_recursive(namespace, type_name, ref_path)

                self.output_text[namespace].append('struct %s' % (self.to_struct_style(type_name), ))
                self.output_text[namespace].append('{')
                self.output_text[namespace].append('    // union start : %s' % (type_name, ))
                for ref_path in obj.get('items', {}).get('refs', []):
                    (ref_namespace, ref_type_name) = self.split_ref(ref_path)
                    if len(ref_type_name) == 0:
                        ref_type_name = 'main'
                    if len(ref_namespace) == 0:
                        extend_ns = '%s::' % (self.to_namespace_style(namespace), )
                    else:
                        extend_ns = '%s::' % (self.to_namespace_style(ref_namespace), )
                    self.output_text[namespace].append('                QList<%s%s> %s;' % (extend_ns, self.to_struct_style(ref_type_name),ref_type_name, ))

                self.output_text[namespace].append('    // union end : %s' % (type_name, ))
                self.output_text[namespace].append('};')

            elif items_type == 'ref':
                # refのときはその型を先に処理する
                ref_path = obj.get('items', {}).get('ref', '')
                self.output_ref_recursive(namespace, type_name, ref_path)

                (ref_namespace, ref_type_name) = self.split_ref(ref_path)
                self.output_text[namespace].append('typedef QList<%s> %s;%s' % (self.to_struct_style(ref_type_name), self.to_struct_style(type_name), obj_comment, ))

            elif items_type == 'integer':
                self.output_text[namespace].append('typedef QList<int> %s;%s' % (self.to_struct_style(type_name), obj_comment, ))
            elif items_type == 'boolean':
                self.output_text[namespace].append('typedef QList<bool> %s;%s' % (self.to_struct_style(type_name), obj_comment, ))
            elif items_type == 'string':
                self.output_text[namespace].append('typedef QList<QString> %s;%s' % (self.to_struct_style(type_name), obj_comment, ))
            self.append_history(namespace, type_name)

        else:
            variant_key = obj.get('type', '')
            variant_obj = obj.get(variant_key)
            if variant_obj is not None:
                self.output_type(namespace, type_name, variant_obj)

            # assert obj.get('type') == 'object', 'A type that is not a candidate for structure.'

        self.namespace_stack.pop()

    def check_pointer(self,  namespace: str, type_name: str, property_name: str, ref_namespace: str, ref_type_name):
        if len(ref_namespace) == 0:
            return (namespace + '#' + type_name + '#' + property_name + '#' + namespace + '#' + ref_type_name in self.history_pointer)
        else:
            return (namespace + '#' + type_name + '#' + property_name + '#' + ref_namespace + '#' + ref_type_name in self.history_pointer)

    def check_object(self, ref_namespace: str, name: str) -> bool:
        func_defs = self.history_func.get(ref_namespace)
        if func_defs is None:
            return True
        temp_name = 'void %s(' % (name, )
        for func_def in func_defs:
            if temp_name in func_def:
                if 'const QJsonValue &src' in func_def:
                    return False
        return True

    def output_function(self, namespace: str, type_name: str, obj: dict):

        if obj.get('type') == 'object':
            # 構造体

            if namespace not in self.output_func_text:
                self.output_func_text[namespace] = []

            function_define = 'void copy%s(const QJsonObject &src, %s::%s &dest)' % (
                self.to_struct_style(type_name), self.to_namespace_style(namespace), self.to_struct_style(type_name), )
            self.output_func_text[namespace].append(function_define)
            self.output_func_text[namespace].append('{')
            self.output_func_text[namespace].append('    if (!src.isEmpty()) {')

            # if (!json_author.isEmpty()) {
            #     author.avatar = json_author.value("avatar").toString();

            properties = obj.get('properties', {})
            for property_name in properties.keys():
                if self.check_deprecated(properties[property_name]):
                    continue
                p_type = properties[property_name].get('type')
                if p_type == 'ref':
                    ref_path = properties[property_name].get('ref', {})
                    (ref_namespace, ref_type_name) = self.split_ref(ref_path)
                    if len(ref_type_name) == 0:
                        ref_type_name = 'main'
                    if len(ref_namespace) == 0:
                        extend_ns = ''
                        forward_type = self.history_type[namespace + '#' + ref_type_name]
                    else:
                        extend_ns = '%s::' % (self.to_namespace_style(ref_namespace), )
                        forward_type = self.history_type[ref_namespace + '#' + ref_type_name]
                    if self.check_pointer(namespace, type_name, property_name, ref_namespace, ref_type_name):
                        self.output_func_text[namespace].append('        if (dest.%s.isNull())' % (property_name, ))
                        self.output_func_text[namespace].append('            dest.%s = QSharedPointer<%s%s>(new %s%s());' % (property_name, extend_ns, self.to_struct_style(ref_type_name), extend_ns, self.to_struct_style(ref_type_name), ))
                        self.output_func_text[namespace].append('        %scopy%s(src.value("%s").toObject(), *dest.%s);' % (extend_ns, self.to_struct_style(ref_type_name), property_name, property_name, ))
                    else:
                        if forward_type in ['integer', 'string', 'boolean']:
                            convert_method = ''
                        else:
                            convert_method = '.toObject()'
                        self.output_func_text[namespace].append('        %scopy%s(src.value("%s")%s, dest.%s);' % (
                            extend_ns, self.to_struct_style(ref_type_name), property_name, convert_method, property_name,))

                elif p_type == 'union':
                    value_key: str = '$type'
                    if properties[property_name].get('forceRerative') is not None:
                        value_key = 'type'
                    self.output_func_text[namespace].append('        QString %s_type = src.value("%s").toObject().value("%s").toString();' % (property_name, property_name, value_key, ))
                    for ref_path in properties[property_name].get('refs', []):
                        (ref_namespace, ref_type_name) = self.split_ref(ref_path)
                        if len(ref_type_name) == 0:
                            ref_type_name = 'main'

                        if len(ref_type_name) == 0:
                            self.output_func_text[namespace].append('        // union %s %s' % (property_name, ref_path, ))
                        else:
                            if len(ref_namespace) == 0:
                                extend_ns = '%s::' % (self.to_namespace_style(namespace), )
                                union_name = '%s_%s' % (property_name, self.to_struct_style(ref_type_name))
                                if properties[property_name].get('forceRerative', False):
                                    # plc.directoryの解析専用
                                    ref_path_full = ref_type_name
                                else:
                                    ref_path_full = namespace + '#' + ref_type_name
                            else:
                                extend_ns = '%s::' % (self.to_namespace_style(ref_namespace), )
                                union_name = '%s_%s_%s' % (property_name, self.to_namespace_style(ref_namespace), self.to_struct_style(ref_type_name))
                                ref_path_full = ref_path
                            union_type_name = '%s%sType' % (self.to_struct_style(type_name), self.to_struct_style(property_name), )

                            if self.check_pointer(namespace, type_name, property_name, ref_namespace, ref_type_name):
                                self.output_func_text[namespace].append('        // union *%s %s' % (property_name, ref_path, ))
                                self.output_func_text[namespace].append('        if (%s_type == QStringLiteral("%s")) {' % (property_name, ref_path_full, ))
                                self.output_func_text[namespace].append('            dest.%s_type = %s::%s::%s;' % (property_name, self.to_namespace_style(namespace), union_type_name, union_name, ))
                                self.output_func_text[namespace].append('            if (dest.%s.isNull())' % (union_name, ))
                                self.output_func_text[namespace].append('                dest.%s = QSharedPointer<%s%s>(new %s%s());' % (union_name, extend_ns, self.to_struct_style(ref_type_name), extend_ns, self.to_struct_style(ref_type_name), ))
                                self.output_func_text[namespace].append('            %scopy%s(src.value("%s").toObject(), *dest.%s);' % (extend_ns, self.to_struct_style(ref_type_name), property_name, union_name, ))
                                self.output_func_text[namespace].append('        }')
                            else:
                                self.output_func_text[namespace].append('        if (%s_type == QStringLiteral("%s")) {' % (property_name, ref_path_full, ))
                                self.output_func_text[namespace].append('            dest.%s_type = %s::%s::%s;' % (property_name, self.to_namespace_style(namespace), union_type_name, union_name, ))
                                self.output_func_text[namespace].append('            %scopy%s(src.value("%s").toObject(), dest.%s);' % (extend_ns, self.to_struct_style(ref_type_name), property_name, union_name, ))
                                self.output_func_text[namespace].append('        }')

                elif p_type == 'unknown':
                    self.output_func_text[namespace].append('        LexiconsTypeUnknown::copyUnknown(src.value("%s").toObject(), dest.%s);' % (property_name, property_name, ))

                elif p_type == 'integer':
                    self.output_func_text[namespace].append('        dest.%s = src.value("%s").toInt();' % (property_name, property_name, ))

                elif p_type == 'boolean':
                    self.output_func_text[namespace].append('        dest.%s = src.value("%s").toBool();' % (property_name, property_name, ))

                elif p_type == 'string':
                    self.output_func_text[namespace].append('        dest.%s = src.value("%s").toString();' % (property_name, property_name, ))

                elif p_type == 'array':
                    items_type = properties[property_name].get('items', {}).get('type', '')
                    (ref_namespace, ref_type_name) = self.split_ref(properties[property_name].get('items', {}).get('ref', {}))
                    if len(ref_type_name) == 0:
                        ref_type_name = 'main'
                    if len(ref_namespace) == 0:
                        extend_ns = ''
                    else:
                        extend_ns = '%s::' % (self.to_namespace_style(ref_namespace), )
                    if items_type == 'ref':
                        func_name = '%scopy%s' % (extend_ns, self.to_struct_style(ref_type_name), )
                        self.output_func_text[namespace].append('        for (const auto &s : src.value("%s").toArray()) {' % (property_name, ))
                        if self.check_pointer(namespace, type_name, property_name, ref_namespace, ref_type_name):
                            self.output_func_text[namespace].append('            QSharedPointer<%s%s> child = QSharedPointer<%s%s>(new %s%s());' % (extend_ns, self.to_struct_style(ref_type_name), extend_ns, self.to_struct_style(ref_type_name), extend_ns, self.to_struct_style(ref_type_name), ))
                            self.output_func_text[namespace].append('            %s(s.toObject(), *child);' % (func_name, ))
                            self.output_func_text[namespace].append('            dest.%s.append(child);' % (property_name, ))
                        else:
                            self.output_func_text[namespace].append('            %s%s child;' % (extend_ns, self.to_struct_style(ref_type_name), ))
                            if self.check_object(ref_namespace, 'copy%s' % (self.to_struct_style(ref_type_name), )):
                                self.output_func_text[namespace].append('            %s(s.toObject(), child);' % (func_name, ))
                            else:
                                self.output_func_text[namespace].append('            %s(s, child);' % (func_name, ))
                            self.output_func_text[namespace].append('            dest.%s.append(child);' % (property_name, ))
                        self.output_func_text[namespace].append('        }')

                    elif items_type == 'union':
                        # self.output_union(namespace, type_name, property_name, properties[property_name].get('items', {}).get('refs', []), True)
                        self.output_func_text[namespace].append('        // array<union> ' + property_name)
                        for ref_path in properties[property_name].get('items', {}).get('refs', []):
                            (ref_namespace, ref_type_name) = self.split_ref(ref_path)
                            if len(ref_type_name) == 0:
                                ref_type_name = 'main'

                            if len(ref_type_name) == 0:
                                self.output_func_text[namespace].append('        // union %s %s' % (property_name, ref_path, ))
                            else:
                                if len(ref_namespace) == 0:
                                    extend_ns = '%s::' % (self.to_namespace_style(namespace), )
                                    union_name = '%s_%s' % (property_name, self.to_struct_style(ref_type_name))
                                    ref_path_full = namespace + '#' + ref_type_name
                                else:
                                    extend_ns = '%s::' % (self.to_namespace_style(ref_namespace), )
                                    union_name = '%s_%s_%s' % (property_name, self.to_namespace_style(ref_namespace), self.to_struct_style(ref_type_name))
                                    ref_path_full = ref_path
                                union_type_name = '%s%sType' % (self.to_struct_style(type_name), self.to_struct_style(property_name), )

                                self.output_func_text[namespace].append('        for (const auto &value : src.value("%s").toArray()) {' % (property_name, ))
                                self.output_func_text[namespace].append('            QString value_type = value.toObject().value("$type").toString();')
                                self.output_func_text[namespace].append('            if (value_type == QStringLiteral("%s")) {' % (ref_path_full, ))
                                if self.check_pointer(namespace, type_name, property_name, ref_namespace, ref_type_name):
                                    self.output_func_text[namespace].append('                QSharedPointer<%s%s> child = QSharedPointer<%s%s>(new %s%s());' % (extend_ns, self.to_struct_style(ref_type_name), extend_ns, self.to_struct_style(ref_type_name), extend_ns, self.to_struct_style(ref_type_name), ))
                                    self.output_func_text[namespace].append('                %scopy%s(value.toObject(), *child);' % (extend_ns, self.to_struct_style(ref_type_name), ))
                                    self.output_func_text[namespace].append('                dest.%s.append(child);' % (union_name, ))
                                else:
                                    self.output_func_text[namespace].append('                %s%s child;' % (extend_ns, self.to_struct_style(ref_type_name), ))
                                    self.output_func_text[namespace].append('                %scopy%s(value.toObject(), child);' % (extend_ns, self.to_struct_style(ref_type_name), ))
                                    self.output_func_text[namespace].append('                dest.%s.append(child);' % (union_name, ))
                                self.output_func_text[namespace].append('            }')
                                self.output_func_text[namespace].append('        }')

                    elif items_type == 'integer':
                        self.output_func_text[namespace].append('        for (const auto &value : src.value("%s").toArray()) {' % (property_name, ))
                        self.output_func_text[namespace].append('            dest.%s.append(value.toInt());' % (property_name, ))
                        self.output_func_text[namespace].append('        }')

                    elif items_type == 'boolean':
                        self.output_func_text[namespace].append('        for (const auto &value : src.value("%s").toArray()) {' % (property_name, ))
                        self.output_func_text[namespace].append('            dest.%s.append(value.toBool());' % (property_name, ))
                        self.output_func_text[namespace].append('        }')

                    elif items_type == 'string':
                        self.output_func_text[namespace].append('        for (const auto &value : src.value("%s").toArray()) {' % (property_name, ))
                        self.output_func_text[namespace].append('            dest.%s.append(value.toString());' % (property_name, ))
                        self.output_func_text[namespace].append('        }')
                elif p_type == 'blob':
                    self.output_func_text[namespace].append('        LexiconsTypeUnknown::copyBlob(src.value("%s").toObject(), dest.%s);' % (property_name, property_name, ))
                    # self.output_text[namespace].append('    Blob %s;' % (property_name, ))

            self.output_func_text[namespace].append('    }')
            self.output_func_text[namespace].append('}')

            self.append_func_history(namespace, function_define)

        elif obj.get('type') == 'string':
            # 文字列は型定義にする
            # self.output_text[namespace].append('typedef QString %s;' % (self.to_struct_style(type_name), ))
            if namespace not in self.output_func_text:
                self.output_func_text[namespace] = []

            function_define = 'void copy%s(const QJsonValue &src, %s::%s &dest)' % (
                self.to_struct_style(type_name), self.to_namespace_style(namespace), self.to_struct_style(type_name), )
            self.output_func_text[namespace].append(function_define)
            self.output_func_text[namespace].append('{')
            self.output_func_text[namespace].append('    dest = src.toString();')
            self.output_func_text[namespace].append('}')

            self.append_func_history(namespace, function_define)

        elif obj.get('type') == 'integer':
            # 数値は型定義にする
            # self.output_text[namespace].append('typedef int %s;' % (self.to_struct_style(type_name), ))
            if namespace not in self.output_func_text:
                self.output_func_text[namespace] = []

            function_define = 'void copy%s(const QJsonValue &src, %s::%s &dest)' % (
                self.to_struct_style(type_name), self.to_namespace_style(namespace), self.to_struct_style(type_name), )
            self.output_func_text[namespace].append(function_define)
            self.output_func_text[namespace].append('{')
            self.output_func_text[namespace].append('    dest = src.toInt();')
            self.output_func_text[namespace].append('}')

            self.append_func_history(namespace, function_define)

        elif obj.get('type') == 'boolean':
            # bool値は型定義にする
            if namespace not in self.output_func_text:
                self.output_func_text[namespace] = []

            function_define = 'void copy%s(const QJsonValue &src, %s::%s &dest)' % (
                self.to_struct_style(type_name), self.to_namespace_style(namespace), self.to_struct_style(type_name), )
            self.output_func_text[namespace].append(function_define)
            self.output_func_text[namespace].append('{')
            self.output_func_text[namespace].append('    dest = src.toBool();')
            self.output_func_text[namespace].append('}')

            self.append_func_history(namespace, function_define)

        elif obj.get('type') == 'array':
            # array
            if namespace not in self.output_func_text:
                self.output_func_text[namespace] = []
            function_define = 'void copy%s(const QJsonArray &src, %s::%s &dest)' % (
                self.to_struct_style(type_name), self.to_namespace_style(namespace), self.to_struct_style(type_name), )
            self.output_func_text[namespace].append(function_define)
            self.output_func_text[namespace].append('{')
            self.output_func_text[namespace].append('    if (!src.isEmpty()) {')

            items_type = obj.get('items', {}).get('type', '')
            if items_type == 'union':
                self.output_func_text[namespace].append('        for (const auto &value : src) {')
                self.output_func_text[namespace].append('            QString value_type = value.toObject().value("$type").toString();')
                chain_if = ''
                for ref_path in obj.get('items', {}).get('refs', []):
                    (ref_namespace, ref_type_name) = self.split_ref(ref_path)
                    if len(ref_type_name) == 0:
                        ref_type_name = 'main'

                    if len(ref_type_name) == 0:
                        self.output_func_text[namespace].append('        // union %s %s' % (type_name, ref_path, ))
                    else:
                        if len(ref_namespace) == 0:
                            extend_ns = '%s::' % (self.to_namespace_style(namespace), )
                            union_name = '%s_%s' % (type_name, self.to_struct_style(ref_type_name))
                            ref_path_full = namespace + '#' + ref_type_name
                        else:
                            extend_ns = '%s::' % (self.to_namespace_style(ref_namespace), )
                            union_name = '%s_%s_%s' % (type_name, self.to_namespace_style(ref_namespace), self.to_struct_style(ref_type_name))
                            ref_path_full = ref_path
                        union_type_name = '%s%sType' % (self.to_struct_style(type_name), self.to_struct_style(type_name), )

                        self.output_func_text[namespace].append('            %sif (value_type == QStringLiteral("%s")) {' % (chain_if, ref_path_full, ))
                        self.output_func_text[namespace].append('                %s%s child;' % (extend_ns, self.to_struct_style(ref_type_name), ))
                        self.output_func_text[namespace].append('                %scopy%s(value.toObject(), child);' % (extend_ns, self.to_struct_style(ref_type_name), ))
                        self.output_func_text[namespace].append('                dest.%s.append(child);' % (ref_type_name, ))
                        self.output_func_text[namespace].append('            }')
                        if len(chain_if) == 0:
                            chain_if = '             else '
                self.output_func_text[namespace].append('        }')
            elif items_type == 'ref':
                (ref_namespace, ref_type_name) = self.split_ref(obj.get('items', {}).get('ref', {}))
                if len(ref_type_name) == 0:
                    ref_type_name = 'main'
                if len(ref_namespace) == 0:
                    extend_ns = ''
                else:
                    extend_ns = '%s::' % (self.to_namespace_style(ref_namespace), )

                func_name = '%scopy%s' % (extend_ns, self.to_struct_style(ref_type_name), )
                self.output_func_text[namespace].append('        for (const auto &s : src) {')
                if self.check_pointer(namespace, type_name, '', ref_namespace, ref_type_name):
                    self.output_func_text[namespace].append('            QSharedPointer<%s%s> child = QSharedPointer<%s%s>(new %s%s());' % (extend_ns, self.to_struct_style(ref_type_name), extend_ns, self.to_struct_style(ref_type_name), extend_ns, self.to_struct_style(ref_type_name), ))
                    self.output_func_text[namespace].append('            %s(s.toObject(), *child);' % (func_name, ))
                    self.output_func_text[namespace].append('            dest.%s.append(child);' % (property_name, ))
                else:
                    self.output_func_text[namespace].append('            %s%s child;' % (extend_ns, self.to_struct_style(ref_type_name), ))
                    if self.check_object(ref_namespace, 'copy%s' % (self.to_struct_style(ref_type_name), )):
                        self.output_func_text[namespace].append('            %s(s.toObject(), child);' % (func_name, ))
                    else:
                        self.output_func_text[namespace].append('            %s(s, child);' % (func_name, ))
                    self.output_func_text[namespace].append('            dest.append(child);')
                self.output_func_text[namespace].append('        }')


            elif items_type == 'integer':
                pass
            elif items_type == 'boolean':
                pass
            elif items_type == 'string':
                pass

            self.output_func_text[namespace].append('    }')
            self.output_func_text[namespace].append('}')

            self.append_func_history(namespace, function_define)
        else:
            variant_key = obj.get('type', '')
            variant_obj = self.json_obj.get(namespace, {}).get('defs', {}).get('main', {}).get(variant_key, {})
            if variant_obj is not None:
                self.output_function(namespace, type_name, variant_obj)

    def output_api_class_data(self, namespace: str, ref: str, var_type: str, property_name: str, key_name: str) -> dict:
        data: dict = {}
        (ref_namespace, ref_struct_name) = self.split_ref(ref)

        if len(ref_namespace) == 0:
            ref_namespace = namespace

        data['parent_info'] = self.inheritance.get(ref, {})
        data['copy_method'] = 'AtProtocolType::%s::copy%s' % (self.to_namespace_style(ref_namespace),
                                                self.to_struct_style(ref_struct_name), )
        data['variable_is_obj'] = (var_type == 'obj')
        data['variable_is_array'] = var_type.startswith('array_')
        data['variable_is_union'] = (var_type == 'array_union')
        data['variable_key_name'] = key_name
        data['variable_type'] = 'AtProtocolType::%s::%s' % (self.to_namespace_style(ref_namespace),
                                                            self.to_struct_style(ref_struct_name), )
        if var_type.startswith('array_'):
            data['method_getter'] = '%sList' % (property_name, )
            data['variable_name'] = 'm_%sList' % (property_name, )
        else:
            data['method_getter'] = '%s' % (property_name, )
            data['variable_name'] = 'm_%s' % (property_name, )
        if self.history_type.get(ref, '') == 'array':
            data['variable_to'] = '.toArray()'
        else:
            data['variable_to'] = '.toObject()'
        data['union_ref'] = ref

        return data

    def output_api_class_data_primitive(self, pro_type: str, key_name: str) -> dict:
        data: dict = {}

        data['parent_info'] = {}
        data['variable_is_obj'] = False
        data['variable_is_array'] = False
        data['variable_is_union'] = False
        data['variable_key_name'] = key_name
        if pro_type == 'string':
            data['copy_method'] = 'AtProtocolType::LexiconsTypeUnknown::copyString'
            data['variable_type'] = 'QString'
        elif pro_type == 'array_string':
            data['copy_method'] = 'AtProtocolType::LexiconsTypeUnknown::copyStringList'
            data['variable_type'] = 'QStringList'
        elif pro_type == 'boolean':
            data['copy_method'] = 'AtProtocolType::LexiconsTypeUnknown::copyBool'
            data['variable_type'] = 'bool'
        elif pro_type == 'integer':
            data['copy_method'] = 'AtProtocolType::LexiconsTypeUnknown::copyInt'
            data['variable_type'] = 'int'
        elif pro_type == 'blob':
            data['copy_method'] = 'AtProtocolType::LexiconsTypeUnknown::copyBlob'
            data['variable_type'] = 'Blob'
        elif pro_type == 'byte_array':
            data['copy_method'] = 'AtProtocolType::LexiconsTypeUnknown::copyByteArray'
            data['variable_type'] = 'QByteArray'
        elif pro_type == 'unknown':
            data['copy_method'] = 'AtProtocolType::LexiconsTypeUnknown::copyUnknown'
            data['variable_type'] = 'QVariant'
        if pro_type.startswith('array_'):
            data['method_getter'] = '%sList' % (key_name, )
            data['variable_name'] = 'm_%sList' % (key_name, )
            data['variable_to'] = '.toArray()'
        else:
            data['method_getter'] = '%s' % (key_name, )
            data['variable_name'] = 'm_%s' % (key_name, )
            if pro_type == 'unknown':
                data['variable_to'] = '.toObject()'
            else:
                data['variable_to'] = ''
        return data

    def output_api_class_data_union(self, namespace: str, union_ref: str, key_name: str) -> dict:
        data: dict = {}
        (ref_namespace, ref_struct_name) = self.split_ref(union_ref)

        if len(ref_namespace) == 0:
            ref_namespace = namespace
        if len(ref_struct_name) == 0:
            ref_struct_name = 'main'

        data['parent_info'] = self.inheritance.get(union_ref, {})
        data['copy_method'] = 'AtProtocolType::%s::copy%s' % (self.to_namespace_style(ref_namespace),
                                                self.to_struct_style(ref_struct_name), )
        data['variable_is_obj'] = False
        data['variable_is_array'] = False
        data['variable_is_union'] = True
        data['variable_key_name'] = key_name
        data['variable_type'] = 'AtProtocolType::%s::%s' % (self.to_namespace_style(ref_namespace),
                                                            self.to_struct_style(ref_struct_name), )
        data['method_getter'] = '%s' % (ref_struct_name, )
        data['variable_name'] = 'm_%s' % (ref_struct_name, )
        data['variable_to'] = '.toObject()'
        data['union_ref'] = union_ref

        return data

    def output_api_class_raw_header(self, namespace: str) -> dict:
        data: list = []

        for key in self.rawHttpHeader:
            if namespace.startswith(key):
                data.append(self.rawHttpHeader.get(key))

        return data


    def output_api_class(self, namespace: str, type_name: str):
        obj = self.get_defs_obj(namespace, type_name)
        data: dict = {}
        if obj.get('type') == 'query' or obj.get('type') == 'procedure':
            data['file_name_lower'] = namespace.replace('.', '').lower()
            data['file_name_upper'] = namespace.replace('.', '').upper()
            data['method_name'] = namespace.split('.')[-1]
            data['class_name'] = self.to_namespace_style(namespace)
            data['parent_class_name'] = 'AccessAtProtocol'
            data['include_paths'] = ['atprotocol/accessatprotocol.h']
            data['user_auth'] = (namespace not in self.unuse_auth)
            arguments: list[FunctionArgument] = []
            if obj.get('type') == 'query':
                # query
                data['access_type'] = 'get'
                properties = obj.get('parameters', {}).get('properties')
                if properties is not None:
                    for pro_name, pro_value in properties.items():
                        if self.check_deprecated(pro_value):
                            continue

                        pro_type = pro_value.get('type', '')
                        if pro_type == 'array':
                            pro_type = pro_value.get('items', {}).get('type', '')
                            arguments.append(FunctionArgument(pro_type, pro_name, True))
                        else:
                            arguments.append(FunctionArgument(pro_type, pro_name, False))
            elif obj.get('type') == 'procedure':
                # post
                data['access_type'] = 'post'
                properties = obj.get('input', {}).get('schema', {}).get('properties')
                if properties is not None:
                    for pro_name, pro_value in properties.items():
                        if self.check_deprecated(pro_value):
                            continue
                        pro_type = pro_value.get('type', '')
                        if pro_type == 'array':
                            pro_type = pro_value.get('items', {}).get('type', '')
                            if pro_type == 'ref':
                                pro_ref = pro_value.get('items', {}).get('ref', '')
                                if pro_ref.startswith('#'):
                                    pro_ref = namespace + pro_ref
                                arguments.append(FunctionArgument('json_' + self.history_type.get(pro_ref, ''), pro_name, False))
                            else:
                                arguments.append(FunctionArgument(pro_type, pro_name, True))
                        elif pro_type == 'ref':
                            pro_ref = pro_value.get('ref', '')
                            arguments.append(FunctionArgument('json_' + self.history_type.get(pro_ref, ''), pro_name, False))
                        elif pro_type == 'union':
                            pro_refs = pro_value.get('refs', [])
                            for pro_ref in pro_refs:
                                if pro_ref in self.history_type:
                                    pro_type = self.history_type.get(pro_ref)
                                    break
                            arguments.append(FunctionArgument(pro_type, pro_name, False))
                        elif pro_type == 'ref':
                            pass
                        else:
                            arguments.append(FunctionArgument(pro_type, pro_name, False))

            args: list[str] = []
            query: list[str] = []
            payload: list[str] = []
            for argument in arguments:
                args.append(argument.to_string_arg())
                query.append(argument.to_string_query())
                payload.append(argument.to_string_payload())

            data['method_args'] = ','.join(args)
            data['method_query'] = ''.join(query)
            if len(payload) > 0:
                data['method_payload'] = ''.join(payload)
            data['api_id'] = namespace

        if obj.get('output') is not None:
            schema = obj.get('output', {}).get('schema', {})
            encoding = obj.get('output', {}).get('encoding', {})
            if encoding == '*/*':
                variables = (('byte_array', 'blobData', ), ('string', 'extension', ), )
                for variable in variables:
                    item_obj = self.output_api_class_data_primitive(variable[0], variable[1])
                    if len(item_obj) > 0:
                        item_obj['variable_is_obj'] = True
                        data['members'] = data.get('members', [])
                        data['members'].append(item_obj)
                        data['has_primitive'] = True
                data['recv_image'] = True

            elif encoding == 'application/vnd.ipld.car':
                pass
            elif encoding != 'application/json':
                pass
            elif schema.get('type', '') == 'ref':
                ref = schema.get('ref', '')
                (ref_namespace, ref_key_name) = self.split_ref(ref)
                item_obj = self.output_api_class_data(namespace, ref, '', ref_key_name, '')
                if len(item_obj) > 0:
                    data['members'] = data.get('members', [])
                    data['members'].append(item_obj)

            elif schema.get('type', '') == 'object':
                for key_name, property_obj in schema.get('properties', {}).items():
                    pro_type = property_obj.get('type')
                    pro_items = property_obj.get('items', {})
                    (ref_namespace, ref_struct_name) = self.split_ref(pro_items.get('ref', ''))
                    if pro_type == 'array':
                        item_type = pro_items.get('type')
                        if item_type == 'ref':
                            var_type = 'array_ref'
                        elif item_type == 'union':
                            var_type = 'array_union'
                        elif item_type == 'string':
                            var_type = 'array_string'
                        else:
                            var_type = ''
                            print (namespace + ":" + ref_namespace + "," + ref_struct_name + " not ref")

                        if var_type == 'array_string':
                            item_obj = self.output_api_class_data_primitive(var_type, key_name)
                            if len(item_obj) > 0:
                                item_obj['variable_is_obj'] = True
                                data['members'] = data.get('members', [])
                                data['members'].append(item_obj)
                                data['is_parent'] = False
                                data['has_parent_class'] = False
                                data['has_primitive'] = True
                        elif len(var_type) > 0:
                            refs = pro_items.get('refs', [])
                            if len(refs) == 0 and 'ref' in pro_items:
                                refs = [pro_items.get('ref', '')]
                            if len(refs) == 0:
                                refs = [namespace]
                            for ref in refs:
                                (var_ref_namespace, var_ref_key_name) = self.split_ref(ref)
                                if len(refs) > 1:
                                    item_obj = self.output_api_class_data(namespace, ref, var_type, key_name + self.to_struct_style(var_ref_key_name), key_name)
                                else:
                                    item_obj = self.output_api_class_data(namespace, ref, var_type, key_name, key_name)
                                if len(item_obj) > 0:
                                    data['members'] = data.get('members', [])
                                    data['members'].append(item_obj)
                                    if len(item_obj['parent_info']) > 0:
                                        parent_class_name = self.to_namespace_style(item_obj['parent_info']['parent_namespace'])
                                        if parent_class_name != data['class_name']:
                                            data['is_parent'] = False
                                            data['has_parent_class'] = True
                                            data['parent_class_name'] = parent_class_name
                                            data['include_paths'] = [self.to_header_path(item_obj['parent_info']['parent_namespace'])]
                                        else:
                                            data['is_parent'] = True
                                            data['has_parent_class'] = False
                                    else:
                                        data['is_parent'] = False
                                        data['has_parent_class'] = False
                                else:
                                    print (namespace + ":" + ref_namespace + "," + ref_struct_name + " ??")
                    elif pro_type == 'string' or pro_type == 'boolean' or pro_type == 'integer' or pro_type == 'blob' or pro_type == 'unknown':
                        if key_name == 'cursor':
                            data['has_cursor'] = True
                        else:
                            item_obj = self.output_api_class_data_primitive(pro_type, key_name)
                            if len(item_obj) > 0:
                                item_obj['variable_is_obj'] = True
                                data['members'] = data.get('members', [])
                                data['members'].append(item_obj)
                                data['has_primitive'] = True
                    elif pro_type == 'ref':
                        ref = property_obj.get('ref', '')
                        item_obj = self.output_api_class_data(namespace, ref, 'obj', key_name, key_name)
                        if len(item_obj) > 0:
                            data['members'] = data.get('members', [])
                            data['members'].append(item_obj)
                    elif pro_type == 'union':
                        union_refs = property_obj.get('refs', [])
                        for union_ref in union_refs:
                            item_obj = self.output_api_class_data_union(namespace, union_ref, key_name)
                            if len(item_obj) > 0:
                                data['members'] = data.get('members', [])
                                data['members'].append(item_obj)
                    else:
                        print (namespace + ":" + ref_namespace + "," + ref_struct_name + " not array")

        if len(data) > 0:
            data['recv_image'] = data.get('recv_image', False)
            data['has_primitive'] = data.get('has_primitive', False)
            data['has_parent_class'] = data.get('has_parent_class', False)
            data['raw_headers'] = self.output_api_class_raw_header(namespace)
            data['completed'] = True
            data['my_include_path'] = self.to_header_path(namespace)
            data['need_extension'] = (namespace in self.need_extension)
            # if data.get('access_type', '') == 'post':
            #     data['completed'] = False
            self.api_class[namespace] = data




    def output_file_lexicons_h(self, environment, output_path: str):
        """ lexicons.h """
        template = environment.get_template('template/lexicons.h.j2')

        params = {}
        params['pre_defines'] = []
        for namespace in sorted(self.pre_define.keys()):
            structs = []
            for type_name in sorted(self.pre_define[namespace]):
                structs.append({'name': type_name})
            params['pre_defines'].append({'namespace': namespace, 'structs': structs})

        params['defines'] = []
        for namespace in self.history_namespace:
            structs = []
            for text in self.output_text[namespace]:
                structs.append({'line': text})
            params['defines'].append({'namespace_org': namespace, 'namespace': self.to_namespace_style(namespace), 'structs': structs})

        params['metatypes'] = []
        for name in self.metatype:
            params['metatypes'].append({'name': name})

        with open(output_path + '/lexicons.h', 'w', encoding='utf-8') as fp:
            fp.write(template.render(params))

    def output_lexicons_func_cpp(self, environment, output_path: str):
        """ lexicons_func.cpp """
        template = environment.get_template('template/lexicons_func.cpp.j2')

        params = {}
        params['functions'] = []
        for namespace, func_lines in self.output_func_text.items():
            lines = []
            for line in func_lines:
                lines.append({'line': line})
            params['functions'].append({'namespace_org': namespace, 'namespace': self.to_namespace_style(namespace), 'func_lines': lines})

        with open(output_path + '/lexicons_func.cpp', 'w', encoding='utf-8') as fp:
            fp.write(template.render(params))

    def output_lexicons_func_h(self, environment, output_path: str):
        """ lexicons_func.h """
        template = environment.get_template('template/lexicons_func.h.j2')

        params = {}
        params['functions'] = []
        for namespace, function_defines in self.history_func.items():
            lines = []
            for function_define in function_defines:
                lines.append({'line': function_define})
            params['functions'].append({'namespace_org': namespace, 'namespace': self.to_namespace_style(namespace), 'func_lines': lines})

        with open(output_path + '/lexicons_func.h', 'w', encoding='utf-8') as fp:
            fp.write(template.render(params))

    def output_api_class_cpp_h(self, environment, output_path: str):

        template = environment.get_template('template/api_class.h.j2')
        for namespace, value in self.api_class.items():
            if value['completed']:
                output_folder = os.path.join(output_path, '/'.join(namespace.split('.')[:-1]))
            else:
                output_folder = os.path.join(os.path.dirname(__file__), 'out', '/'.join(namespace.split('.')[:-1]))

            os.makedirs(output_folder, exist_ok=True)

            with open(os.path.join(output_folder, value['file_name_lower'] + '.h'), 'w', encoding='utf-8') as fp:
                fp.write(template.render(value))

        template = environment.get_template('template/api_class.cpp.j2')
        for namespace, value in self.api_class.items():
            if value['completed']:
                output_folder = os.path.join(output_path, '/'.join(namespace.split('.')[:-1]))
            else:
                output_folder = os.path.join(os.path.dirname(__file__), 'out', '/'.join(namespace.split('.')[:-1]))

            with open(os.path.join(output_folder, value['file_name_lower'] + '.cpp'), 'w', encoding='utf-8') as fp:
                fp.write(template.render(value))


        template = environment.get_template('template/api_class_ex.h.j2')
        for namespace, value in self.api_class.items():
            if not value['need_extension']:
                continue
            output_folder = os.path.join(os.path.dirname(__file__), 'out_ex', '/'.join(namespace.split('.')[:-1]))
            os.makedirs(output_folder, exist_ok=True)

            with open(os.path.join(output_folder, value['file_name_lower'] + 'ex.h'), 'w', encoding='utf-8') as fp:
                fp.write(template.render(value))

        template = environment.get_template('template/api_class_ex.cpp.j2')
        for namespace, value in self.api_class.items():
            if not value['need_extension']:
                continue
            output_folder = os.path.join(os.path.dirname(__file__), 'out_ex', '/'.join(namespace.split('.')[:-1]))

            with open(os.path.join(output_folder, value['file_name_lower'] + 'ex.cpp'), 'w', encoding='utf-8') as fp:
                fp.write(template.render(value))



    def output(self, output_path: str) -> None:
        # 各ファイル（名前空間）ごとに解析する
        for namespace, type_obj in self.json_obj.items():
            if namespace not in self.output_text:
                self.output_text[namespace] = []

            description = self.json_obj[namespace].get('description', '')
            if len(description) > 0:
                self.output_text[namespace].append('// %s' % (description, ))

            defs = self.json_obj[namespace].get('defs', {})
            for type_name in defs.keys():
                self.output_type(namespace, type_name, self.get_defs_obj(namespace, type_name))

        for namespace, type_obj in self.json_obj.items():
            if not self.skip_spi_class(namespace):
                # class
                defs = self.json_obj[namespace].get('defs', {})
                for type_name in defs.keys():
                    self.output_api_class(namespace, type_name)


        # コピー関数のための解析
        for ref_path in self.history:
            (ref_namespace, ref_type_name) = self.split_ref(ref_path)
            obj = self.get_defs_obj(ref_namespace, ref_type_name)
            self.output_function(ref_namespace, ref_type_name, obj)


        # jinja2を使用してコードを出力
        environment = Environment(loader=FileSystemLoader(os.path.dirname(__file__), encoding='utf8'))
        self.output_file_lexicons_h(environment, output_path)
        self.output_lexicons_func_cpp(environment, output_path)
        self.output_lexicons_func_h(environment, output_path)
        self.output_api_class_cpp_h(environment, output_path)

    def json_deep_merge(self, dest, src):
        """ 再帰的に辞書をマージ """
        for key in src:
            if key in dest:
                if isinstance(dest[key], dict) and isinstance(src[key], dict):
                    self.json_deep_merge(dest[key], src[key])
                elif isinstance(dest[key], list) and isinstance(src[key], list):
                    dest[key].extend(src[key])
            else:
                dest[key] = src[key]
        return dest

    def open(self, lexicons_path: str, base_path: str, is_extend: bool = False) -> None:
        obj = None
        with open(lexicons_path, 'r') as fp:
            obj = json.load(fp)
        if obj is None:
            print('Failed to load json file')
            return
        rel_path = lexicons_path.removeprefix(base_path).removesuffix('.json')
        if rel_path[0] == '/':
            rel_path = rel_path[1:]
        namespace = rel_path.replace('/', '.')

        if not is_extend:
            extend_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'lexicons', namespace + '.json')
            if os.path.isfile(extend_path):
                # lexiconのユーザー拡張のJSONファイルがあれば合体する
                with open(extend_path, 'r') as fp:
                    extend_obj = json.load(fp)
                    obj = self.json_deep_merge(obj, extend_obj)
                self.converted_extend_paths.append(extend_path)

        self.json_obj[namespace] = obj

def main(lexicons_path: str, output_path: str) -> None:

    def2struct = Defs2Struct()
    #atproto\lexicons
    lexicons_path = lexicons_path.replace('\\', '/')
    file_list = glob.glob(lexicons_path + '/**/*.json', recursive=True)
    for file in file_list:
        def2struct.open(file.replace('\\', '/'), lexicons_path)

    # 既存のlexiconに合体していないファイルを処理
    extend_base_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'lexicons')
    file_list = glob.glob(extend_base_path + '/*.json')
    for file in file_list:
        if file not in def2struct.converted_extend_paths:
            def2struct.open(file.replace('\\', '/'), extend_base_path.replace('\\', '/'), True)

    def2struct.output(output_path)

if __name__ == "__main__":

    base_dir = os.path.dirname(__file__)
    lexicons_path: str = os.path.join(base_dir, '..', '1stparty', 'atproto', 'lexicons')
    output: str = os.path.join(base_dir, '..', 'lib', 'atprotocol')

    main(lexicons_path, output)
