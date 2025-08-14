# BasicLang

BasicLang is a minimal, experimental esoteric language designed for simplicity and readability while still being quirky enough to stand out.  
It supports basic variables, loops, conditionals, printing, and program exit codes — all with a playful syntax.

## ✨ Syntax Overview

- **Variable Declaration**
  ```bl
  imagine x = 10;
  ```

- **Loop**
  ```bl
  doit (n) { ... }
  ```
  Runs the block `n` times.

- **Conditional**
  ```bl
  is (condition) { ... } else { ... }
  ```

- **Print**
  ```bl
  print("Hello World");
  ```

- **Exit Program**
  ```bl
  yeet 0;
  ```


## 📦 Installation & Setup

```bash
# Clone the repository
git clone https://github.com/raghavendrashande/esolang.git
cd esolang

# Build 
cmake -S . -B build
cmake --build build
```

---

## ▶ Usage

```bash
# Run a BasicLang program
./basiclang path/to/file.bl
```

---

## 📝 Example Program (`test.bl`)

```bl
imagine x = 1;

doit (10) {
    is ((x % 2) == 0) {
        print("even");
    } else {
        print("odd");
    }
    x = x + 1;
}

yeet 255;
```

**Output**:
```
odd
even
odd
even
odd
even
odd
even
odd
even
```

**Exit Code**: `255`

---

## 📜 License

MIT License — see [LICENSE](LICENSE) for details.
