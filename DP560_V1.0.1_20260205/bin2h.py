import os
def bin_to_hex_array(bin_file_path, output_h_file_path, array_name="bin_data"):
    """
    将bin文件转换为C语言的头文件，其中包含一个十六进制数组，每16个元素换行
    :param bin_file_path: 输入的bin文件路径
    :param output_h_file_path: 输出的头文件路径
    :param array_name: 数组的名称，默认为"bin_data"
    """
    try:
        # 打开bin文件并读取内容
        with open(bin_file_path, "rb") as bin_file:
            bin_data = bin_file.read()
        
        # 将二进制数据转换为十六进制字符串数组
        hex_array = []
        for i, byte in enumerate(bin_data):
            if i % 16 == 0 and i != 0:
                hex_array.append(",\n    ")
            hex_array.append(f"0x{byte:02X}")
            if i % 16 != 15:
                hex_array.append(", ")
        
        # 将数组内容拼接成字符串
        hex_array_str = "".join(hex_array)
        
        # 创建C语言头文件内容
        h_file_content = f"""#include <stdint.h>

uint8_t firmware_pid1[] = {{
    {hex_array_str}
}};


uint8_t firmware_pid2[] = {{
    {hex_array_str}
}};
"""
        
        # 将内容写入头文件
        with open(output_h_file_path, "w") as h_file:
            h_file.write(h_file_content)
        print(f"成功生成头文件：{output_h_file_path}")
    except Exception as e:
        print(f"发生错误：{e}")

def process_bin_files():
    """
    处理当前目录下的所有bin文件，生成对应的h文件
    """
    files = [f for f in os.listdir('.') if os.path.isfile(f)]
    bin_files = [f for f in files if f.endswith('.bin')]
    if not bin_files:
        print("警告：当前目录下没有找到任何bin文件。")
        return
    for bin_file in bin_files:
        output_h_file = f"cts_fw.h"
        bin_to_hex_array(bin_file, output_h_file, bin_file[:-4])
if __name__ == "__main__":


    process_bin_files()

