from base import g_cache, GPT
from flask import Flask, request
import os

model = "gemma2:2b"
gpt = GPT(model, "xx", "http://localhost:11434/v1/")

script_dir = os.path.dirname(os.path.abspath(__file__))
prompt_file = os.path.join(script_dir, 'prompt.json')

g_prompt = {}

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
    
    answer = gpt.gpt_35_api(m).replace("\n", "").strip()
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
           
if __name__ == '__main__':
    app.run(host="0.0.0.0", port=5100)
    # 非流式调用
    # gpt_35_api(messages)
    # 流式调用
    # gpt_35_api_stream(messages)