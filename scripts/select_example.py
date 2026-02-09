Import("env")
import os

custom_src = env.GetProjectOption("custom_src_dir", None)
if custom_src:
    env['PROJECT_SRC_DIR'] = os.path.join(env['PROJECT_DIR'], custom_src)