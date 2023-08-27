import json

def deep_merge(dict1, dict2):
    """再帰的に辞書をマージする"""
    for key in dict2:
        if key in dict1:
            # 両方の辞書にキーが存在し、それらの値が辞書である場合、再帰的にマージ
            if isinstance(dict1[key], dict) and isinstance(dict2[key], dict):
                deep_merge(dict1[key], dict2[key])
            # dict1の値を優先して保持（何もしない）
        else:
            # dict1にないキーを追加
            dict1[key] = dict2[key]
    return dict1

def merge_json_files(file1, file2):
    with open(file1, 'r') as f1:
        data1 = json.load(f1)

    with open(file2, 'r') as f2:
        data2 = json.load(f2)

    return deep_merge(data1, data2)

# マージしたい2つのJSONファイルのパスを指定
file1_path = './scripts/lexicons/app.bsky.feed.post_org.json'
file2_path = './scripts/lexicons/app.bsky.feed.post.json'

# マージ処理を実行
merged_data = merge_json_files(file1_path, file2_path)

# 結果を表示
print(json.dumps(merged_data))
