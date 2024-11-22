from openai import OpenAI
import os
import json
import atexit

script_dir = os.path.dirname(os.path.abspath(__file__))
cache_file = os.path.join(script_dir, 'cache.json')

g_cache = {}

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

class GPT:
    def __init__(self, model, api_key, base_url) -> None:
        self.model = model
        self.client = OpenAI(api_key=api_key, base_url=base_url)
      
    def gpt_35_api(self, messages: list):
        completion = self.client.chat.completions.create(model=self.model, messages=messages)
        content = completion.choices[0].message.content
        return content
        

    def gpt_35_api_stream(self, messages: list):
        stream = self.client.chat.completions.create(
            model=self.model,
            messages=messages,
            stream=True,
        )
        for chunk in stream:
            if chunk.choices[0].delta.content is not None:
                print(chunk.choices[0].delta.content, end="")

def str_to_bool(s):
    if s in {"True", "False"}:
        return eval(s)
    else:
        raise ValueError(f"Invalid boolean string: {s}")
