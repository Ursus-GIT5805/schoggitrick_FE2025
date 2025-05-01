host=fe

init:
	uv venv -p 3.9
	uv pip install -r requirements.txt

push:
	rsync -av --exclude 'build' --exclude 'sample' * fe:
