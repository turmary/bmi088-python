
## Usage
To generate debian package under folder deobj/
```bash
# preparation
sudo apt-get install debhelper dh-make debmake devscripts

# packaging
# using git tree lastest files
./tools/packaging_deb.sh
# Or if there is git uncommited file changes
# (using working copy files, not tree in git repo)
USE_GIT=no ./tools/packaging_deb.sh
```
