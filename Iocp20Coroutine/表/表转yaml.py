#pip install pandas pyyaml
#pip install openpyxl
import pandas as pd
import yaml
import os

def parse_column_name(col_name):
    """解析列名，支持嵌套结构，如 '坐标1.X', '坐标1.Y', '坐标1.Z'"""
    if '.' in col_name:
        parts = col_name.split('.')
        return parts[0], parts[1]
    return col_name, None

def build_nested_dict(row, columns):
    """构建嵌套字典结构"""
    result = {}
    
    for col in columns:
        parent_key, child_key = parse_column_name(col)
        value = row[col] if pd.notna(row[col]) else ''
        
        if child_key is None:
            # 简单字段
            result[parent_key] = value
        else:
            # 嵌套字段
            if parent_key not in result:
                result[parent_key] = {}
            result[parent_key][child_key] = value
    
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
output_dir = '..\\x64\\Debug\\配置'
excel_to_yaml(excel_file_path, output_dir)

# 等待用户按下回车键
input("按回车键继续...")