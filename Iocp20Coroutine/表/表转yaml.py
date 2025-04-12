import pandas as pd
import yaml

def excel_to_yaml(excel_file_path):
    # 读取Excel文件并获取所有工作表名称
    xls = pd.ExcelFile(excel_file_path)
    sheet_names = xls.sheet_names
    
    # 遍历每个工作表
    for sheet_name in sheet_names:
        # 将工作表读取为DataFrame，第一行作为表头
        df = pd.read_excel(excel_file_path, sheet_name=sheet_name)
        
        # 获取列名
        columns = df.columns
        
        # 将DataFrame转换为字典列表，确保列顺序
        data_list = []
        for _, row in df.iterrows():
            data_dict = {col: row[col] for col in columns}
            data_list.append(data_dict)
        
        # 定义YAML文件路径，基于工作表名称
        yaml_file_path = f'{sheet_name}.yaml'
        
        # 将字典列表写入YAML文件
        with open(yaml_file_path, 'w', encoding='utf-8') as yaml_file:
            yaml.dump(data_list, yaml_file, default_flow_style=False, allow_unicode=True)

# 示例用法
excel_file_path = '单位.xlsx'
excel_to_yaml(excel_file_path)