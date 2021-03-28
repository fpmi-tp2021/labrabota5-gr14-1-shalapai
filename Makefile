main: main.o admin.o user.o mkout
	gcc -o main main.o admin_scenario.o user_scenario.o -l sqlite3
	mv -f *.o obj
	mv -f main bin
main.o: src/main.c
	gcc -c src/main.c
admin.o: src/admin_scenario.c
	gcc -c src/admin_scenario.c
user.o: src/user_scenario.c
	gcc -c src/user_scenario.c
clean:
	rm -rf bin obj
check:
	echo "Nothing to check"
distcheck:
	echo "nothing to distcheck"
mkout:
	mkdir -p bin obj
