Import("env")

# Remove --erase-all flag to preserve WiFi credentials in NVS
# This script modifies the upload command to NOT erase NVS partition

def before_upload(source, target, env):
    upload_flags = env.get("UPLOADERFLAGS", [])
    # Remove any erase flags that would wipe NVS
    if "--erase-all" in upload_flags:
        upload_flags.remove("--erase-all")
    if "--erase-flash" in upload_flags:
        upload_flags.remove("--erase-flash")
    env.Replace(UPLOADERFLAGS=upload_flags)
    print("Upload configured to preserve WiFi credentials")

env.AddPreAction("upload", before_upload)
