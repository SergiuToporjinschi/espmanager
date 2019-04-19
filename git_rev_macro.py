import subprocess

revision = subprocess.check_output(["git", "rev-parse", "HEAD"]).strip()
print "-D REV=\"\\\"%s\\\"\"" % revision