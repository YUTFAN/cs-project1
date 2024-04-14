EXE=allocate

$(EXE): main.c
	cc -Wall -o $(EXE) main.c -lm  # 添加了 -lm 来链接数学库

format:
	clang-format -style=file -i *.c

clean:
	rm -f $(EXE)
