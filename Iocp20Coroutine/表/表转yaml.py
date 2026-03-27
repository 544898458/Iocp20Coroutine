#pip install pandas pyyaml
#pip install openpyxl
import pandas as pd
import yaml
import os
import re
import numpy as np

def convert_to_native_type(value):
    """将 numpy 类型转换为 Python 原生类型"""
    if pd.isna(value):
        return None
    if isinstance(value, (np.integer, np.int64, np.int32, np.int16, np.int8)):
        return int(value)
    if isinstance(value, (np.floating, np.float64, np.float32, np.float16)):
        # 如果浮点数实际上是整数（小数部分为0），转换为整数
        float_value = float(value)
        if float_value.is_integer():
            return int(float_value)
        return float_value
    if isinstance(value, np.bool_):
        return bool(value)
    if isinstance(value, np.ndarray):
        return value.tolist()
    # 处理 Python 原生的 float 类型
    if isinstance(value, float):
        if value.is_integer():
            return int(value)
    return value

# 数组列名格式：ArrayName[index].FieldName，如 Space[0].战局, Space[1].半径
_ARRAY_COL_PATTERN = re.compile(r'^(.+)\[(\d+)\]\.(.+)$')

def parse_column_name(col_name):
    """解析列名。返回 (kind, ...)：'array' -> (array_name, index, field)；'nested' -> (parent, child)；'simple' -> (name, None)"""
    m = _ARRAY_COL_PATTERN.match(col_name)
    if m:
        return ('array', m.group(1), int(m.group(2)), m.group(3))
    if '.' in col_name:
        parts = col_name.split('.', 1)
        return ('nested', parts[0], parts[1])
    return ('simple', col_name, None)

def build_nested_dict(row, columns):
    """构建嵌套字典结构，支持数组字段（如 Space[0].战局, Space[1].半径）"""
    result = {}
    # 先收集数组列：array_name -> { index -> { field -> value } }
    array_data = {}

    for col in columns:
        parsed = parse_column_name(col)
        value = convert_to_native_type(row[col])

        if parsed[0] == 'array':
            _, array_name, index, field = parsed
            if array_name not in array_data:
                array_data[array_name] = {}
            if index not in array_data[array_name]:
                array_data[array_name][index] = {}
            array_data[array_name][index][field] = value
        elif parsed[0] == 'nested':
            _, parent_key, child_key = parsed
            if parent_key not in result:
                result[parent_key] = {}
            result[parent_key][child_key] = value
        else:
            # simple
            result[parsed[1]] = value

    # 将数组转为有序列表写入 result
    for array_name, index_to_obj in array_data.items():
        result[array_name] = [index_to_obj[i] for i in sorted(index_to_obj.keys())]

    return result

def excel_to_yaml(excel_file_path, output_dir):
    # 确保输出目录存在
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # 读取Excel文件并获取所有工作表名称
    xls = pd.ExcelFile(excel_file_path)
    sheet_names = xls.sheet_names
    
    # 遍历每个工作表
    for sheet_name in sheet_names:
        try:
            # 将工作表读取为DataFrame，第一行作为表头
            df = pd.read_excel(excel_file_path, sheet_name=sheet_name)
            
            # 获取列名
            columns = df.columns
            
            # 将DataFrame转换为字典列表，支持嵌套结构
            data_list = []
            for _, row in df.iterrows():
                nested_dict = build_nested_dict(row, columns)
                data_list.append(nested_dict)
            
            # 打印当前工作表的内容
            print(f"处理工作表: {sheet_name}")
            for item in data_list:
                print(item)
            
            # 定义YAML文件路径，基于工作表名称
            yaml_file_path = os.path.join(output_dir, f'{sheet_name}.yaml')
            
            # 将字典列表写入YAML文件
            with open(yaml_file_path, 'w', encoding='utf-8') as yaml_file:
                yaml.dump(data_list, yaml_file, default_flow_style=False, allow_unicode=True)
            
            # 打印成功消息
            print(f"工作表 {sheet_name} 成功转换为 {yaml_file_path}")
        
        except Exception as e:
            # 打印失败消息
            print(f"工作表 {sheet_name} 转换失败: {e}")

# 示例用法
excel_file_path = '单位.xlsx'
output_dir = '../x64/Debug/配置'
excel_to_yaml(excel_file_path, output_dir)

# 等待用户按下回车键
input("按回车键继续...")
