*while true ; do git status && make fclean && git add . && git commit -m "Auto push via script" && git push ; make all ; sleep 180 ; done*
