from openai import OpenAI
from flask import Flask, request
import os
import json
import atexit
from dotenv import load_dotenv

load_dotenv()
api_key = os.getenv("API_KEY")
base_url = os.getenv("BASE_URL")

client = OpenAI(
    api_key=api_key,
    base_url=base_url,
)

model="gpt-3.5-turbo"
model="gpt-4o-mini"

script_dir = os.path.dirname(os.path.abspath(__file__))
cache_file = os.path.join(script_dir, 'cache.json')
g_cache = {}

try:
    with open(cache_file, 'r', encoding='utf-8') as f:
        g_cache = json.load(f)
except FileNotFoundError:
    # 如果文件不存在, 将 g_cache 设为一个空字典
    g_cache = {}

def save_cache():
    with open(cache_file, 'w', encoding='utf-8') as f:
        json.dump(g_cache, f, ensure_ascii=False, indent=4)

# 注册 exit 函数, 在程序退出时调用
atexit.register(save_cache)


app = Flask(__name__)
messages = [
      {
        'role': 'user',
        'content': '''You need to help me correct the lines in '<Two and a Half Men>' Season 3.
        For example, 'Welluh,thanksforyourhospitality' 
        should be corrected to 'Well uh, thanks for your hospitality.'
        For example, 'You wantto fliomethebird,butthepoor little fella can'tfly' 
        should be corrected to 'You want to flip me the bird, but the poor little fella can't fly.' 
        don't be reply with other text or punctuation. Also, and Use English punctuation,
        do not change or add the original words. now, don't be reply with other text expect correct centent.''',
      },
      {
        "role": "user",
        "content": "Welluh,thanksforyourhospitality"
      },
      {
        "role": "assistant",
        "content": '''Well uh, thanks for your hospitality.'''
      },
    ]


def repair(content: str, no_cache: bool):
    m = messages[:]
    m.append({
      "role": "user",
      "content": content,
    })
    
    if content in g_cache and no_cache == False:
        return g_cache[content]
    
    g_cache[content] = gpt_35_api(m)
    return g_cache[content]

@app.route('/', methods=['GET'])
def index():
    content = request.args.get("content")
    no_cache = request.args.get("no_cache", False)
    # content = content.replace("%20", "")

    r = repair(content, no_cache)
    print("repair ->", r, "before ->", content)
    return r

# 非流式响应
def gpt_35_api(messages: list):
    completion = client.chat.completions.create(model=model, messages=messages)
    content = completion.choices[0].message.content
    return content
    

def gpt_35_api_stream(messages: list):
    stream = client.chat.completions.create(
        model=model,
        messages=messages,
        stream=True,
    )
    for chunk in stream:
        if chunk.choices[0].delta.content is not None:
            print(chunk.choices[0].delta.content, end="")
            
if __name__ == '__main__':
    app.run(host="0.0.0.0", port=5100)
    # 非流式调用
    # gpt_35_api(messages)
    # 流式调用
    # gpt_35_api_stream(messages)