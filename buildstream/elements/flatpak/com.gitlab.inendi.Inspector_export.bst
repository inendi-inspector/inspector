kind: manual

sources:
  - kind: local
    path: buildstream/files

build-depends:
- base.bst
# - inendi-inspector-cleanup.bst

config:
  install-commands:
  - install -D -m 0644 "com.gitlab.inendi.Inspector.png" -t "%{install-root}%{prefix}/share/icons/hicolor/128x128/apps/"
  - install -D -m 0644 "com.gitlab.inendi.Inspector.png" -t "%{install-root}%{prefix}/share/app-info/icons/flatpak/128x128/"
  - |
    cat <<EOF >com.gitlab.inendi.Inspector.desktop
    [Desktop Entry]
    Type=Application
    Name=INENDI Inspector
    Exec=inspector_launcher.sh
    Icon=com.gitlab.inendi.Inspector
    Categories=Science,Graphics;Network;
    EOF
    install -D -m 0644 "com.gitlab.inendi.Inspector.desktop" -t "%{install-root}%{prefix}/share/applications"
  - install -D -m 0644 "com.gitlab.inendi.Inspector.metainfo.xml" -t "%{install-root}%{prefix}/share/metainfo"
