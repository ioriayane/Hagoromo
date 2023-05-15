# レキシコンのjsonから構造体の定義するヘッダーを出力するツール
# https://github.com/bluesky-social/atproto/tree/main/lexicons
# query系のoutputからの参照を起点に定義を作った方が良いかもしれない

# python3 ./scripts/defs2struct.py ../atproto/lexicons/ ./app/atprotocol

import sys
import os
import glob
import json
from jinja2 import Template, Environment, FileSystemLoader

class FuncHistoryItem:
    def __init__(self, type_name, qt_type) -> None:
        self.type_name = type_name
        self.qt_type = qt_type

class Defs2Struct:
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

        self.api_class = {}

        # QVariantに入れる構造体をQ_DECLARE_METATYPE()で定義する構造体
        # （単純にすべてではない）
        self.metatype = ('AppBskyFeedPost::Record', 
                         'AppBskyFeedLike::Record', )

    def to_struct_style(self, name: str) -> str:
        return name[0].upper() + name[1:]

    def to_namespace_style(self, name: str) -> str:
        srcs = name.split('.')
        dest = []
        for src in srcs:
            dest.append(src[0].upper() + src[1:])
            # app.bsky.embed.recordWithMediaがあるのでcapitalize()は使えない
        return ''.join(dest)

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

    def output_ref(self, namespace: str, type_name: str, property_name: str, ref_obj: dict, is_array: bool = False):
        (ref_namespace, ref_struct_name) = self.split_ref(ref_obj)
        if len(ref_struct_name) == 0:
            ref_struct_name = 'main'

        extend_symbol = ''
        if len(ref_namespace) == 0:
            if not is_array:
                if (namespace + '#' + ref_struct_name) in self.namespace_stack:
                    extend_symbol = ' *'
                    init_value = ' = nullptr'
                    self.append_pre_define(namespace, ref_struct_name)
                    self.history_pointer.append(namespace + '#' + type_name + '#' + property_name + '#' + namespace + '#' + ref_struct_name)
                else:
                    extend_symbol = ' '
                    init_value = ''
                self.output_text[namespace].append('    %s%s%s%s;' % (
                    self.to_struct_style(ref_struct_name), extend_symbol, property_name, init_value
                    ))
            else:
                self.output_text[namespace].append('    QList<%s> %s;' % (
                    self.to_struct_style(ref_struct_name), property_name
                    ))
        else:
            # 履歴で自分の方が前にいるということは宣言が後ろなのでポインタにする
            # 自分が履歴にいないときは宣言が前
            my_history_pos = self.index_of(self.history_namespace, namespace)
            ref_history_pos = self.index_of(self.history_namespace, ref_namespace)
            if not is_array:
                if (ref_namespace + '#' + ref_struct_name) in self.namespace_stack or (my_history_pos < ref_history_pos):
                    extend_symbol = ' *'
                    init_value = ' = nullptr'
                    self.append_pre_define(ref_namespace, ref_struct_name)
                    self.history_pointer.append(namespace + '#' + type_name + '#' + property_name + '#' + ref_namespace + '#' + ref_struct_name)
                else:
                    extend_symbol = ' '
                    init_value = ''
                self.output_text[namespace].append('    %s::%s%s%s%s;' % (
                    self.to_namespace_style(ref_namespace), self.to_struct_style(ref_struct_name), extend_symbol, property_name, init_value
                    ))
            else:
                self.output_text[namespace].append('    QList<%s::%s> %s;' % (
                    self.to_namespace_style(ref_namespace), self.to_struct_style(ref_struct_name), property_name
                    ))


    def output_union(self, namespace: str, type_name: str, property_name: str, refs_obj: dict, is_array: bool = False):
        pointer_list = []
        enum_text = []
        union_name_list = []
        self.output_text[namespace].append('    // union start : %s' % (property_name, ))
        pos = len(self.output_text[namespace])
        for ref in refs_obj:
            (ref_namespace, ref_struct_name) = self.split_ref(ref)
            if len(ref_struct_name) == 0:
                self.output_text[namespace].append('    //     union=%s' % (ref_namespace, ))
            else:
                extend_symbol = ''
                if len(ref_namespace) == 0:
                    union_name = '%s_%s' % (property_name, self.to_struct_style(ref_struct_name))
                    union_name_list.append(union_name)
                    if not is_array:
                        if (namespace + '#' + ref_struct_name) in self.namespace_stack:
                            extend_symbol = ' *'
                            init_value = ' = nullptr'
                            pointer_list.append(union_name)
                            self.append_pre_define(namespace, ref_struct_name)
                            self.history_pointer.append(namespace + '#' + type_name + '#' + property_name + '#' + namespace + '#' + ref_struct_name)
                        else:
                            extend_symbol = ' '
                            init_value = ''
                        self.output_text[namespace].append('    %s%s%s%s;' % (
                            self.to_struct_style(ref_struct_name), extend_symbol, union_name, init_value
                            ))
                    else:
                        self.output_text[namespace].append('    QList<%s> %s;' % (
                            self.to_struct_style(ref_struct_name), union_name
                            ))
                else:
                    union_name = '%s_%s_%s' % (property_name, self.to_namespace_style(ref_namespace), self.to_struct_style(ref_struct_name))
                    union_name_list.append(union_name)
                    if not is_array:
                        if (ref_namespace + '#' + ref_struct_name) in self.namespace_stack:
                            extend_symbol = ' *'
                            init_value = ' = nullptr'
                            pointer_list.append(union_name)
                            self.append_pre_define(ref_namespace, ref_struct_name)
                            self.history_pointer.append(namespace + '#' + type_name + '#' + property_name + '#' + ref_namespace + '#' + ref_struct_name)
                        else:
                            extend_symbol = ' '
                            init_value = ''
                        self.output_text[namespace].append('    %s::%s%s%s%s;' % (
                            self.to_namespace_style(ref_namespace), self.to_struct_style(ref_struct_name), extend_symbol, union_name, init_value
                            ))
                    else:
                        self.output_text[namespace].append('    QList<%s::%s> %s;' % (
                            self.to_namespace_style(ref_namespace), self.to_struct_style(ref_struct_name), union_name
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

        return (pointer_list, enum_text)

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
            pointer_list = []
            enum_text = []
            styled_type_name = self.to_struct_style(type_name)
            self.output_text[namespace].append('struct %s' % (styled_type_name, ))
            self.output_text[namespace].append('{')
            pos = len(self.output_text[namespace])
            for property_name in properties.keys():
                p_type = properties[property_name].get('type')
                if p_type == 'ref':
                    self.output_ref(namespace, type_name, property_name, properties[property_name].get('ref', {}))
                elif p_type == 'union':
                    (temp_pointer, temp_enum) = self.output_union(namespace, type_name, property_name, properties[property_name].get('refs', []))
                    pointer_list.extend(temp_pointer)
                    enum_text.extend(temp_enum)
                elif p_type == 'unknown':
                    self.output_text[namespace].append('    QVariant %s;' % (property_name, ))
                elif p_type == 'integer':
                    self.output_text[namespace].append('    int %s = 0;' % (property_name, ))
                elif p_type == 'string':
                    comment = properties[property_name].get('format', '')
                    if len(comment) > 0:
                        self.output_text[namespace].append('    QString %s; // %s' % (property_name, comment))
                    else:
                        self.output_text[namespace].append('    QString %s; //' % (property_name, ))
                elif p_type == 'array':
                    items_type = properties[property_name].get('items', {}).get('type', '')
                    if items_type == 'ref':
                        self.output_ref(namespace, type_name, property_name, properties[property_name].get('items', {}).get('ref', {}), True)
                    elif items_type == 'union':
                        (temp_pointer, temp_enum) = self.output_union(namespace, type_name, property_name, properties[property_name].get('items', {}).get('refs', []), True)
                        pointer_list.extend(temp_pointer)
                        enum_text.extend(temp_enum)

            # ポインタを含む構造体はconstructorなどを追加する
            if len(pointer_list) > 0:
                self.output_text[namespace].insert(pos, '    %s() { }' % (styled_type_name, ))
                pos += 1
                self.output_text[namespace].insert(pos, '    %s(const %s &) = delete;' % (styled_type_name, styled_type_name, ))
                pos += 1
                self.output_text[namespace].insert(pos, '    ~%s()' % styled_type_name)
                pos += 1
                self.output_text[namespace].insert(pos, '    {')
                pos += 1
                for pointer_name in pointer_list:
                    self.output_text[namespace].insert(pos, '        if (%s != nullptr)' % (pointer_name, ))
                    pos += 1
                    self.output_text[namespace].insert(pos, '            delete %s;' % (pointer_name, ))
                    pos += 1
                self.output_text[namespace].insert(pos, '    }')
                pos += 1
                self.output_text[namespace].insert(pos, '    %s &operator=(const %s &) = delete;' % (styled_type_name, styled_type_name, ))

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
            self.output_text[namespace].append('typedef QString %s;' % (self.to_struct_style(type_name), ))
            self.append_history(namespace, type_name)

        elif obj.get('type') == 'integer':
            # 数値は型定義にする
            self.output_text[namespace].append('typedef int %s;' % (self.to_struct_style(type_name), ))
            self.append_history(namespace, type_name)

        else:
            variant_key = obj.get('type', '')
            variant_obj = obj.get(variant_key)
            if variant_obj is not None:
                self.output_type(namespace, variant_key, variant_obj)

            # assert obj.get('type') == 'object', 'A type that is not a candidate for structure.'

        self.namespace_stack.pop()

    def check_pointer(self,  namespace: str, type_name: str, property_name: str, ref_namespace: str, ref_type_name):
        if len(ref_namespace) == 0:
            return (namespace + '#' + type_name + '#' + property_name + '#' + namespace + '#' + ref_type_name in self.history_pointer)
        else:
            return (namespace + '#' + type_name + '#' + property_name + '#' + ref_namespace + '#' + ref_type_name in self.history_pointer)

    def output_function(self, namespace: str, type_name: str):
        
        obj = self.get_defs_obj(namespace, type_name)
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
                p_type = properties[property_name].get('type')
                if p_type == 'ref':
                    ref_path = properties[property_name].get('ref', {})
                    (ref_namespace, ref_type_name) = self.split_ref(ref_path)
                    if len(ref_type_name) == 0:
                        self.output_func_text[namespace].append('        // ref %s %s' % (property_name, ref_path, ))
                    elif self.check_pointer(namespace, type_name, property_name, ref_namespace, ref_type_name):
                        self.output_func_text[namespace].append('        // ref *%s %s' % (property_name, ref_path, ))
                    else:
                        if len(ref_namespace) == 0:
                            extend_ns = ''
                            forward_type = self.history_type[namespace + '#' + ref_type_name]
                        else:
                            extend_ns = '%s::' % (self.to_namespace_style(ref_namespace), )
                            forward_type = self.history_type[ref_namespace + '#' + ref_type_name]
                        if forward_type in ['integer', 'string']:
                            convert_method = ''
                        else:
                            convert_method = '.toObject()'
                        self.output_func_text[namespace].append('        %scopy%s(src.value("%s")%s, dest.%s);' % (
                            extend_ns, self.to_struct_style(ref_type_name), property_name, convert_method, property_name,))

                elif p_type == 'union':
                    self.output_func_text[namespace].append('        QString %s_type = src.value("%s").toObject().value("$type").toString();' % (property_name, property_name, ))
                    for ref_path in properties[property_name].get('refs', []):
                        (ref_namespace, ref_type_name) = self.split_ref(ref_path)
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

                            if self.check_pointer(namespace, type_name, property_name, ref_namespace, ref_type_name):
                                self.output_func_text[namespace].append('        // union *%s %s' % (property_name, ref_path, ))
                                self.output_func_text[namespace].append('        if (%s_type == QStringLiteral("%s")) {' % (property_name, ref_path_full, ))
                                self.output_func_text[namespace].append('            if (dest.%s == nullptr)' % (union_name, ))
                                self.output_func_text[namespace].append('                dest.%s = new %s;' % (union_name, self.to_struct_style(ref_type_name), ))
                                self.output_func_text[namespace].append('            dest.%s_type = %s::%s::%s;' % (property_name, self.to_namespace_style(namespace), union_type_name, union_name, ))
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

                elif p_type == 'string':
                    self.output_func_text[namespace].append('        dest.%s = src.value("%s").toString();' % (property_name, property_name, ))

                elif p_type == 'array':
                    items_type = properties[property_name].get('items', {}).get('type', '')
                    (ref_namespace, ref_type_name) = self.split_ref(properties[property_name].get('items', {}).get('ref', {}))
                    if len(ref_namespace) == 0:
                        extend_ns = ''
                    else:
                        extend_ns = '%s::' % (self.to_namespace_style(ref_namespace), )
                    if items_type == 'ref':
                        self.output_func_text[namespace].append('        for (const auto &s : src.value("%s").toArray()) {' % (property_name, ))
                        self.output_func_text[namespace].append('            %s%s child;' % (extend_ns, self.to_struct_style(ref_type_name), ))
                        self.output_func_text[namespace].append('            %scopy%s(s.toObject(), child);' % (extend_ns, self.to_struct_style(ref_type_name), ))
                        self.output_func_text[namespace].append('            dest.%s.append(child);' % (property_name, )) 
                        self.output_func_text[namespace].append('        }')

                    elif items_type == 'union':
                        # self.output_union(namespace, type_name, property_name, properties[property_name].get('items', {}).get('refs', []), True)
                        self.output_func_text[namespace].append('        // array<union> ' + property_name)

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

    def output_api_class(self, namespace: str, type_name: str):
        obj = self.get_defs_obj(namespace, type_name)
        if obj.get('type') == 'query' or obj.get('type') == 'procedure':
            data = {}
            data['file_name_lower'] = namespace.replace('.', '').lower()
            data['file_name_upper'] = namespace.replace('.', '').upper()
            data['method_name'] = namespace.split('.')[-1]
            data['class_name'] = self.to_namespace_style(namespace)
            args = ''
            # query
            properties = obj.get('parameters', {}).get('properties')
            if properties is not None:
                data['access_type'] = 'get'
                for pro_name, pro_value in properties.items():
                    pro_type = pro_value.get('type', '')
                    if pro_type == 'string':
                        if len(args) > 0:
                            args += ", "
                        args += "const QString &%s" % (pro_name, )
                    elif pro_type == 'integer':
                        if len(args) > 0:
                            args += ", "
                        args += "const int %s" % (pro_name, )
                    elif pro_type == 'array':
                        pro_type = pro_value.get('items', {}).get('type', '')
                        if pro_type == 'string':
                            if len(args) > 0:
                                args += ", "
                            args += "const QList<QString> &%s" % (pro_name, )
                        elif pro_type == 'integer':
                            if len(args) > 0:
                                args += ", "
                            args += "const QList<int> %s" % (pro_name, )
            # post
            properties = obj.get('input', {}).get('schema', {}).get('properties')
            if properties is not None:
                data['access_type'] = 'post'
                for pro_name, pro_value in properties.items():
                    pro_type = pro_value.get('type', '')
                    if pro_type == 'string':
                        if len(args) > 0:
                            args += ", "
                        args += "const QString &%s" % (pro_name, )
                    elif pro_type == 'integer':
                        if len(args) > 0:
                            args += ", "
                        args += "const int %s" % (pro_name, )
            data['method_args'] = args
            data['api_id'] = namespace
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
            output_folder = os.path.join(output_path, '/'.join(namespace.split('.')[:-1]))
            os.makedirs(output_folder, exist_ok=True)

            with open(os.path.join(output_folder, value['file_name_lower'] + '.h'), 'w', encoding='utf-8') as fp:
                fp.write(template.render(value))

        template = environment.get_template('template/api_class.cpp.j2')
        for namespace, value in self.api_class.items():
            output_folder = os.path.join(output_path, '/'.join(namespace.split('.')[:-1]))
            with open(os.path.join(output_folder, value['file_name_lower'] + '.cpp'), 'w', encoding='utf-8') as fp:
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

                self.output_api_class(namespace, type_name)


        # コピー関数のための解析
        for ref_path in self.history:
            (ref_namespace, ref_type_name) = self.split_ref(ref_path)
            self.output_function(ref_namespace, ref_type_name)


        # jinja2を使用してコードを出力
        environment = Environment(loader=FileSystemLoader(os.path.dirname(__file__), encoding='utf8'))
        self.output_file_lexicons_h(environment, output_path)
        self.output_lexicons_func_cpp(environment, output_path)
        self.output_lexicons_func_h(environment, output_path)
        self.output_api_class_cpp_h(environment, os.path.join(os.path.dirname(__file__), 'out'))


    def open(self, lexicons_path: str, base_path: str) -> None:
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

        self.json_obj[namespace] = obj

def main(lexicons_path: str, output_path: str) -> None:

    def2struct = Defs2Struct()
    #atproto\lexicons
    lexicons_path = lexicons_path.replace('\\', '/')
    file_list = glob.glob(lexicons_path + '/**/*.json', recursive=True)
    for file in file_list:
        def2struct.open(file.replace('\\', '/'), lexicons_path)

    def2struct.output(output_path)

if __name__ == "__main__":

    if len(sys.argv) != 3:
        print('usage: python defs2struct.py path/to/atproto/lexicons path/to/lexicons.h')
        print('   atproto/lexicons : https://github.com/bluesky-social/atproto/tree/main/lexicons')
        exit(1)

    lexicons_path: str = sys.argv[1]
    output: str = sys.argv[2]

    main(lexicons_path, output)
