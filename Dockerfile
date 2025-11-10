# Dockerfile
FROM ubuntu:22.04

# نصب پکیج‌ها
RUN apt-get update && apt-get install -y \
    mingw-w64 \
    wine \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY *.cpp *.h ./

# کامپایل هر دو برنامه
RUN x86_64-w64-mingw32-g++-posix -static -O2 -o main.exe main.cpp SharedMemory.cpp \
    -lws2_32 -luser32 -lkernel32 -ladvapi32
    
RUN x86_64-w64-mingw32-g++-posix -static -O2 -o child.exe child.cpp SharedMemory.cpp \
    -lws2_32 -luser32 -lkernel32 -ladvapi32

CMD ["wine", "main.exe"]