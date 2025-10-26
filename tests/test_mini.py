import re
import subprocess
import random
from typing import List

testcases_len = 13

tests = [f"testcase/test{i + 1}.in" for i in range(testcases_len)]


def get_testcase(idx: int):
    test = tests[idx]
    # print(test)
    return subprocess.run(["cat", test], stdout=subprocess.PIPE, stderr=subprocess.PIPE)


def gen_output(idx: int):
    res = subprocess.run(
        ["./mini1.out"],
        input=get_testcase(idx).stdout,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    asm_out = res.stdout.decode()
    with open(f"output/output{idx + 1}.result", "w") as f:
        f.write(asm_out)


def validate_output(idx: int):
    with open(f"output/output{idx + 1}.result", "r") as f:
        all = f.readlines()
        return not bool(re.findall(r"Compile", "".join(all)))


def compile_generated_output(idx: int) -> bool:
    asmc_out = ""
    randomint = [str(random.randint(-100, 100)) for _ in range(3)]
    if validate_output(idx):
        with open(f"output/output{idx + 1}.result", "r") as f:
            asm = "".join(f.readlines())
            if idx == 3:
                randomint = ["51", "20", "23"]
            asmc = subprocess.run(
                ["./ASMC.out"] + randomint,
                input=asm.encode(),
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            asmc_out = asmc.stdout.decode()
    return check_asmc_output(output=asmc_out, randomint=randomint, idx=idx)


def check_asmc_output(output: str, randomint: List[str], idx: int) -> bool:
    check = subprocess.run(
        ["./test.out"] + randomint + [str(idx + 1)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    result = check.stdout.decode()
    # print(f"#{idx} - {output}")
    # print(re.findall(r"x, y, z = .*", result))
    # print(re.findall(r"x, y, z = .*", output))
    print(re.findall(r".*[Cc]ycle.*", output))
    if not output:
        return bool(re.findall(r"res = -1", result))
    else:
        return bool(re.findall(r"res = 0", result)) and bool(
            re.findall(r"x, y, z = .*", result) == re.findall(r"x, y, z = .*", output)
        )


def test():
    subprocess.run(["make", "check"])
    subprocess.run(["make", "mini"])
    print()
    for i in range(testcases_len):
        gen_output(i)
        assert compile_generated_output(i)


if __name__ == "__main__":
    test()
