from openai import OpenAI
from flask import Flask, request
import os
import json
import atexit

client = OpenAI(
    api_key="xx",
    base_url="http://localhost:11434/v1/",
)
model = "gemma2:2b"

script_dir = os.path.dirname(os.path.abspath(__file__))
cache_file = os.path.join(script_dir, 'cache.json')
prompt_file = os.path.join(script_dir, 'prompt.json')

g_cache = {}
g_prompt = {}

with open(prompt_file, 'r') as file:
    g_prompt = json.load(file)

try:
    with open(cache_file, 'r', encoding='utf-8') as f:
        g_cache = json.load(f)
except FileNotFoundError:
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
    ]
for original, modified in g_prompt.items():
    messages.append({
        "role": "user",
        "content": f"{original}"
    })
    messages.append({
        "role": "assistant",
        "content": f"{modified}"
    })


def repair(content: str, no_cache: bool):
    m = messages[:]
    m.append({
      "role": "user",
      "content": f"sentence: {content}",
    })
    
    if content in g_cache and no_cache == False:
        return g_cache[content]
    
    answer = gpt_35_api(m).replace("\n", "").strip()
    if abs(len(answer.replace(" ", "")) - len(content.replace(" ", ""))) > 5:
        return content
    
    g_cache[content] = answer
    return answer

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