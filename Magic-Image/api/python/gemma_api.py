from base import g_cache, GPT, str_to_bool
import atexit
from OCR import get_ocr
from flask import Flask, request
import os

model = "gemma2:2b"
gpt = GPT(model, "xx", "http://localhost:11434/v1/")
ocr = get_ocr(r"D:/code/Magic-Image/PaddleOCR-json_v1.4.1/PaddleOCR-json.exe")
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
        "content": f'''sentence: "{original}"'''
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
  
@app.route('/ocr', methods=['GET'])
def ocr_get():
    return '''
            <form action="/ocr" method="POST" enctype="multipart/form-data">
              <input type="file" name="img">
              <button type="submit">Up</button>
            </form>
            '''

@app.route('/ocr', methods=['POST'])
def ocr_post():
    no_cache = request.form.get("no_cache", False, str_to_bool)

    img = request.files.get('img', None)
    content = ""
    if img:
        image_bytes = img.read()
        res = ocr.runBytes(image_bytes)
        for line in res["data"]:
            content += line["text"]
    r = repair(content, no_cache)
    print("repair ->", r, "before ->", content)
    return r

           
if __name__ == '__main__':
    app.run(host="0.0.0.0", port=5100)
