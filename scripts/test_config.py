#!/usr/bin/env python3
"""
测试PlatformIO配置解析脚本
用于验证GitHub Actions中的配置检查逻辑
"""

import json
import subprocess
import sys

def test_pio_config():
    """测试PlatformIO配置解析"""
    try:
        # 运行PlatformIO配置命令
        result = subprocess.run(
            ['pio', 'project', 'config', '--json-output'],
            capture_output=True,
            text=True,
            check=True
        )
        
        # 解析JSON输出
        config = json.loads(result.stdout)
        
        print("PlatformIO Configuration Test")
        print("=" * 40)
        print(f"Config type: {type(config)}")
        print(f"Config keys: {list(config.keys()) if isinstance(config, dict) else 'Not a dict'}")
        
        # 查找环境
        environments = []
        if isinstance(config, dict):
            for key in config.keys():
                if key.startswith('env:'):
                    env_name = key[4:]  # 移除 'env:' 前缀
                    environments.append(env_name)
                    print(f"Found environment: {env_name}")
        
        if environments:
            print(f"\nTotal environments found: {len(environments)}")
            for env in environments:
                print(f"  - {env}")
        else:
            print("\nNo environments found")
            
        return True
        
    except subprocess.CalledProcessError as e:
        print(f"Error running PlatformIO: {e}")
        print(f"stdout: {e.stdout}")
        print(f"stderr: {e.stderr}")
        return False
    except json.JSONDecodeError as e:
        print(f"Error parsing JSON: {e}")
        return False
    except Exception as e:
        print(f"Unexpected error: {e}")
        return False

def test_json_parsing():
    """测试JSON解析逻辑"""
    print("\nTesting JSON parsing logic")
    print("=" * 40)
    
    # 模拟PlatformIO配置输出
    test_configs = [
        # 正常的配置格式
        {
            "env:airm2m_core_esp32c3": {
                "platform": "espressif32",
                "board": "airm2m_core_esp32c3"
            },
            "env:esp32dev": {
                "platform": "espressif32", 
                "board": "esp32dev"
            }
        },
        # 空配置
        {},
        # 列表格式（可能的错误情况）
        []
    ]
    
    for i, config in enumerate(test_configs):
        print(f"\nTest case {i + 1}:")
        print(f"Config: {config}")
        print(f"Type: {type(config)}")
        
        if isinstance(config, dict):
            environments = [key[4:] for key in config.keys() if key.startswith('env:')]
            print(f"Environments: {environments}")
        else:
            print("Not a dictionary - no environments found")

if __name__ == "__main__":
    print("PlatformIO Configuration Test Script")
    print("=" * 50)
    
    # 测试JSON解析逻辑
    test_json_parsing()
    
    # 测试实际的PlatformIO配置（如果可用）
    print("\n" + "=" * 50)
    if test_pio_config():
        print("\n✅ PlatformIO configuration test passed")
        sys.exit(0)
    else:
        print("\n❌ PlatformIO configuration test failed")
        sys.exit(1)
