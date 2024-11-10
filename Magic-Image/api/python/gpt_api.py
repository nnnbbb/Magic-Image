            
from flask import Flask, request
from base import g_cache, GPT
from dotenv import load_dotenv
import os
app = Flask(__name__)

script_dir = os.path.dirname(os.path.abspath(__file__))
dotenv_file = os.path.join(script_dir, '.env')

load_dotenv(dotenv_file)
api_key = os.getenv("API_KEY")
base_url = os.getenv("BASE_URL")

model="gpt-3.5-turbo"
model="gpt-4o-mini"
gpt = GPT(model, api_key, base_url)

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
    
    g_cache[content] = gpt.gpt_35_api(m)
    return g_cache[content]

@app.route('/', methods=['GET'])
def index():
    content = request.args.get("content")
    no_cache = request.args.get("no_cache", False)
    # content = content.replace("%20", "")

    r = repair(content, no_cache)
    print("repair ->", r, "before ->", content)
    return r

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=5100)
    # 非流式调用
    # gpt_35_api(messages)
    # 流式调用
    # gpt_35_api_stream(messages)