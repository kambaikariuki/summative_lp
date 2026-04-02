from setuptools import setup, Extension

module = Extension("vibration", sources=["vibration.c"])

setup(
    name="vibration",
    version="1.0",
    description="Vibration analysis C extension",
    ext_modules=[module]
)
