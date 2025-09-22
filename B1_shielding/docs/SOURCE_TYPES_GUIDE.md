# Geant4 GPS 放射源类型详解

## 🎯 支持的放射源类型

使用GPS (General Particle Source)，支持多种放射源类型：

### 1. 位置分布类型 (Position Distribution)

#### 1.1 点源 (Point Source)
```bash
/gps/pos/type Point
/gps/pos/centre 0 0 -30 cm
```

#### 1.2 面源 (Surface Source)
```bash
# 圆形面源
/gps/pos/type Surface
/gps/pos/shape Circle
/gps/pos/radius 2.0 cm

# 矩形面源
/gps/pos/type Surface
/gps/pos/shape Rectangle
/gps/pos/halfx 3.0 cm
/gps/pos/halfy 2.0 cm

# 环形面源
/gps/pos/type Surface
/gps/pos/shape Annulus
/gps/pos/inner_radius 1.0 cm
/gps/pos/outer_radius 3.0 cm
```

#### 1.3 体源 (Volume Source)
```bash
# 球形体源
/gps/pos/type Volume
/gps/pos/shape Sphere
/gps/pos/radius 1.0 cm

# 圆柱体源
/gps/pos/type Volume
/gps/pos/shape Cylinder
/gps/pos/radius 1.0 cm
/gps/pos/halfz 5.0 cm

# 立方体源
/gps/pos/type Volume
/gps/pos/shape Box
/gps/pos/halfx 2.0 cm
/gps/pos/halfy 2.0 cm
/gps/pos/halfz 1.0 cm
```

#### 1.4 平面源 (Plane Source)
```bash
/gps/pos/type Plane
/gps/pos/shape Rectangle
/gps/pos/halfx 2.0 cm
/gps/pos/halfy 2.0 cm
```

### 2. 能量分布类型 (Energy Distribution)

#### 2.1 单能源 (Monoenergetic)
```bash
/gps/ene/type Mono
/gps/ene/mono 0.0595 MeV
```

#### 2.2 连续能谱 (Continuous Spectrum)
```bash
/gps/ene/type Lin
/gps/ene/min 0.01 MeV
/gps/ene/max 0.1 MeV
```

#### 2.3 指数能谱 (Exponential Spectrum)
```bash
/gps/ene/type Exp
/gps/ene/min 0.01 MeV
/gps/ene/max 0.1 MeV
/gps/ene/ezero 0.05 MeV
```

#### 2.4 用户定义能谱 (User-defined Spectrum)
```bash
/gps/ene/type Arb
/gps/hist/type arb
/gps/hist/point 0.030 0.0
/gps/hist/point 0.0595 1.0
/gps/hist/point 0.080 0.0
/gps/hist/inter Lin
```

### 3. 角度分布类型 (Angular Distribution)

#### 3.1 各向同性 (Isotropic)
```bash
/gps/ang/type iso
/gps/ang/mintheta 0 deg
/gps/ang/maxtheta 30 deg
```

#### 3.2 平行束 (Parallel Beam)
```bash
/gps/ang/type beam1d
/gps/ang/sigma_r 0.0 deg
```

#### 3.3 锥形束 (Conical Beam)
```bash
/gps/ang/type beam2d
/gps/ang/sigma_x 5.0 deg
/gps/ang/sigma_y 5.0 deg
```

#### 3.4 用户定义角度 (User-defined Angular)
```bash
/gps/ang/type user
/gps/hist/type theta
/gps/hist/point 0 0
/gps/hist/point 30 1
/gps/hist/point 60 0
```

## 🚀 使用方法

### 基本命令
```bash
# 使用不同的放射源
./build/exampleB1 line_source.mac
./build/exampleB1 surface_source.mac
./build/exampleB1 volume_source.mac
./build/exampleB1 annular_source.mac
./build/exampleB1 multi_energy_source.mac
./build/exampleB1 parallel_beam.mac
./build/exampleB1 conical_beam.mac
```

### 组合使用
```bash
# 中子源 + 面源
PARTICLE_TYPE=neutron ./build/exampleB1 surface_source.mac

# 质子源 + 体源
PARTICLE_TYPE=proton ./build/exampleB1 volume_source.mac
```

## 📊 应用场景

### 1. 点源
- **用途**: 精确的辐射源定位
- **应用**: 剂量校准、点源测量

### 2. 面源
- **用途**: 大面积辐射源
- **应用**: 表面污染测量、大面积屏蔽测试

### 3. 体源
- **用途**: 三维辐射源
- **应用**: 体内辐射源、体积源测量

### 4. 线源
- **用途**: 线性辐射源
- **应用**: 管道内辐射、线性源测量

### 5. 环形源
- **用途**: 环形分布辐射源
- **应用**: 环形加速器、环形源测量

### 6. 平行束
- **用途**: 准直辐射束
- **应用**: 辐射治疗、精确测量

### 7. 锥形束
- **用途**: 发散辐射束
- **应用**: 辐射成像、宽束测量

## ⚙️ 高级配置

### 时间分布
```bash
/gps/time/type Constant
/gps/time/start 0.0 s
/gps/time/stop 1.0 s
```

### 粒子类型
```bash
# 伽马射线
/gps/particle gamma

# 中子
/gps/particle neutron

# 质子
/gps/particle proton

# 电子
/gps/particle e-

# 正电子
/gps/particle e+
```

### 极化
```bash
/gps/polarization 0 0 1
```
