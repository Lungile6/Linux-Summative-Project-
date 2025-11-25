from setuptools import setup, Extension
import numpy

# Define the C extension module
temp_stats_module = Extension(
    'temp_stats',
    sources=['temp_stats.c'],
    include_dirs=[numpy.get_include()]
)

setup(
    name='TemperatureStatistics',
    version='1.0',
    description='A C extension for calculating temperature statistics',
    ext_modules=[temp_stats_module],
)
