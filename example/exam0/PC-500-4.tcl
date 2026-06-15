# =============================================================================
# OpenSees Tcl 版本 — PC-500-4 冲击分析
# 转换自 OpenSeesPy (PC-500-4.ipynb)
# 模型: 2D 钢筋混凝土柱冲击分析
# =============================================================================

wipe
model basic -ndm 2 -ndf 3

# =============================================================================
# 参数 Parameters
# =============================================================================
set impact_mass     0.960          ;# 冲击质量 (ton)
set impact_velocity 4.14           ;# 冲击速度 (m/s)
set P_static        [expr -167.5 * 1000]   ;# 静力荷载 (N)
set Bcol            250            ;# 截面宽度 (mm)
set Hcol            250            ;# 截面高度 (mm)
set c               25             ;# 保护层厚度 (mm)
set d               16             ;# 纵筋直径 (mm)
set pi              [expr acos(-1.0)]

set As              [expr $d * $d * $pi]                ;# 纵筋总面积 (mm^2)
set Ac              [expr 250 * 250 - $As]               ;# 混凝土面积 (mm^2)
set Asv             [expr 2 * 10 * 10 * $pi / 4.0]      ;# 箍筋面积 (mm^2)
set Acor            [expr ($Bcol - 2*$c) * ($Hcol - 2*$c)]

set fy              423            ;# 纵筋静态强度 (MPa)
set fyv             430            ;# 箍筋静态强度 (MPa)
set fc1             28.1           ;# 预制混凝土强度 (MPa)
set fc2             34.8           ;# 现浇混凝土强度 (MPa)

set DIF_fc          1.3
set DIF_fy          1.3
set DIF_P           1.4

set fc1_dyn         [expr $fc1 * $DIF_fc]
set fc2_dyn         [expr $fc2 * $DIF_fc]
set fy_dyn          [expr $fy  * $DIF_fy]

# =============================================================================
# 定义节点 Nodes
# =============================================================================
set ele_mass 0.015

node  1     0.0     0.0   -mass [expr $ele_mass/2]   [expr $ele_mass/2]   [expr $ele_mass/2]
node  2     0.0   100.0   -mass $ele_mass $ele_mass $ele_mass
node  3     0.0   200.0   -mass $ele_mass $ele_mass $ele_mass
node  4     0.0   300.0   -mass [expr $ele_mass/2]   [expr $ele_mass/2]   [expr $ele_mass/2]
node  5     0.0   400.0   -mass $ele_mass $ele_mass $ele_mass
node  6     0.0   500.0   -mass $ele_mass $ele_mass $ele_mass
node  7     0.0   600.0   -mass $ele_mass $ele_mass $ele_mass
node  8     0.0   700.0   -mass $ele_mass $ele_mass $ele_mass
node  9     0.0   800.0   -mass $ele_mass $ele_mass $ele_mass
node 10     0.0   900.0   -mass $ele_mass $ele_mass $ele_mass
node 11     0.0  1000.0   -mass $ele_mass $ele_mass $ele_mass
node 12     0.0  1100.0   -mass $ele_mass $ele_mass $ele_mass
node 13     0.0  1200.0   -mass $ele_mass $ele_mass $ele_mass
node 14     0.0  1300.0   -mass $ele_mass $ele_mass $ele_mass
node 15     0.0  1400.0   -mass $ele_mass $ele_mass $ele_mass
node 16     0.0  1500.0   -mass 0.2075 0.2075 0.2075

node 104    0.0   300.0   -mass [expr $ele_mass/2]   [expr $ele_mass/2]   [expr $ele_mass/2]

node 19     0.0   400.0
node 20     0.0   500.0
node 21     0.0   600.0

node 29     0.0   400.0   -mass [expr $impact_mass/3.0] 0 0
node 30     0.0   500.0   -mass [expr $impact_mass/3.0] 0 0
node 31     0.0   600.0   -mass [expr $impact_mass/3.0] 0 0

# =============================================================================
# 定义节点约束 Constraints
# =============================================================================
equalDOF  4  104  2  3
equalDOF 30   31  1
equalDOF 30   29  1

fix  1   1  1  1

fix 19   0  1  1
fix 20   0  1  1
fix 21   0  1  1

fix 29   0  1  1
fix 30   0  1  1
fix 31   0  1  1

# =============================================================================
# 定义材料 Materials
# =============================================================================

# --- 钢筋 Steel02 ---
set IDSteel 1
set Fy_Steel  $fy_dyn
set E0_Steel  206000
set bs_Steel  0.026
set R0        12.5
set cR1       0.925
set cR2       0.15
uniaxialMaterial Steel02 $IDSteel $Fy_Steel $E0_Steel $bs_Steel $R0 $cR1 $cR2

# --- 混凝土 Concrete04 ---
# 辅助过程: 定义 cover & core 混凝土
proc createConcrete {matTag fpc} {
    set fpc_cover  [expr -$fpc]
    set epsc0_cover [expr -0.002]
    set epsU_cover  [expr -0.004]
    set Ec_cover    [expr 4730 * sqrt(abs($fpc_cover))]
    set fpt_cover   [expr 0.56 * sqrt(abs($fpc_cover))]
    set et_cover    [expr 0.0001 * sqrt(abs($fpc_cover))]
    set beta        0.2

    uniaxialMaterial Concrete04 $matTag $fpc_cover $epsc0_cover $epsU_cover $Ec_cover $fpt_cover $et_cover $beta

    set a         1.0
    # fyv_dyn — use global
    global fyv DIF_fy Asv Acor
    set fyv_dyn  [expr $fyv * $DIF_fy]
    set fpc_core [expr -($fpc + 2*$a*($fyv_dyn * $Asv) / $Acor)]
    set epsc0_core [expr -0.002 * (1 + 5 * $fpc_core / $fpc_cover - 1)]
    set epsU_core  [expr $epsc0_core - 0.02]
    set Ec_core    [expr 4730 * sqrt(abs($fpc_core))]
    set fpt_core   [expr 0.56 * sqrt(abs($fpc_core))]
    set et_core    [expr 0.0001 * sqrt(abs($fpc_core))]
    set beta       0.2

    set coreTag [expr $matTag + 1]
    uniaxialMaterial Concrete04 $coreTag $fpc_core $epsc0_core $epsU_core $Ec_core $fpt_core $et_core $beta

    return $coreTag
}

# Material rebar — tag 1 已定义
# 预制混凝土 (precast): cover=2, core=3
createConcrete 2 $fc1_dyn
# 现浇混凝土 (cast-in-place): cover=4, core=5
createConcrete 4 $fc2_dyn

# =============================================================================
# 定义纤维截面 Fiber Sections
# =============================================================================
set y1col  [expr $Hcol / 2.0]
set z1col  [expr $Bcol / 2.0]
set nFibZ      2
set nFibZc    16
set nFib      20
set nFibCover  2
set nFibCore  16
set As_bar1    [expr $d * $d * $pi / 4.0]
set As_bar2    78.5

# --- 截面 1: 预制混凝土 (precast) ---
section Fiber 1 {
    patch rect 3 $nFibCore $nFibZc  [expr $c - $y1col] [expr $c - $z1col]  [expr $y1col - $c] [expr $z1col - $c]
    patch rect 2 $nFib    $nFibZ   [expr -$y1col]      [expr -$z1col]      $y1col           [expr $c - $z1col]
    patch rect 2 $nFib    $nFibZ   [expr -$y1col]      [expr $z1col - $c]  $y1col           $z1col
    patch rect 2 $nFibCover $nFibZc [expr -$y1col]     [expr $c - $z1col]  [expr $c - $y1col] [expr $z1col - $c]
    patch rect 2 $nFibCover $nFibZc [expr $y1col - $c] [expr $c - $z1col]  $y1col           [expr $z1col - $c]
    layer straight 1 2 $As_bar1  [expr $y1col - $c] [expr $z1col - $c]  [expr $y1col - $c] [expr $c - $z1col]
    layer straight 1 2 $As_bar1  [expr $c - $y1col] [expr $z1col - $c]  [expr $c - $y1col] [expr $c - $z1col]
}

# --- 截面 2: 现浇混凝土 (cast-in-place) ---
section Fiber 2 {
    patch rect 5 $nFibCore $nFibZc  [expr $c - $y1col] [expr $c - $z1col]  [expr $y1col - $c] [expr $z1col - $c]
    patch rect 4 $nFib    $nFibZ   [expr -$y1col]      [expr -$z1col]      $y1col           [expr $c - $z1col]
    patch rect 4 $nFib    $nFibZ   [expr -$y1col]      [expr $z1col - $c]  $y1col           $z1col
    patch rect 4 $nFibCover $nFibZc [expr -$y1col]     [expr $c - $z1col]  [expr $c - $y1col] [expr $z1col - $c]
    patch rect 4 $nFibCover $nFibZc [expr $y1col - $c] [expr $c - $z1col]  $y1col           [expr $z1col - $c]
    layer straight 1 2 $As_bar1  [expr $y1col - $c] [expr $z1col - $c]  [expr $y1col - $c] [expr $c - $z1col]
    layer straight 1 2 $As_bar1  [expr $c - $y1col] [expr $z1col - $c]  [expr $c - $y1col] [expr $c - $z1col]
}

# =============================================================================
# 定义单元 Elements
# =============================================================================
geomTransf Linear 1

# Beam integration: HingeRadau
# beamIntegration HingeRadau tag secI lpI secJ lpJ secE
# element forceBeamColumn eleTag iNode jNode transfTag integrationTag

# 底部单元 1–3: 预制段, 内部截面=2 (secE=2)
beamIntegration HingeRadau 101 1 0 1 0 2
element forceBeamColumn  1   1   2   1  101
element forceBeamColumn  2   2   3   1  101
element forceBeamColumn  3   3   4   1  101

# 单元 4–15: 现浇段, 内部截面=1 (secE=1)
beamIntegration HingeRadau 102 1 0 1 0 1
element forceBeamColumn  4   104   5   1  102
element forceBeamColumn  5     5   6   1  102
element forceBeamColumn  6     6   7   1  102
element forceBeamColumn  7     7   8   1  102
element forceBeamColumn  8     8   9   1  102
element forceBeamColumn  9     9  10   1  102
element forceBeamColumn 10    10  11   1  102
element forceBeamColumn 11    11  12   1  102
element forceBeamColumn 12    12  13   1  102
element forceBeamColumn 13    13  14   1  102
element forceBeamColumn 14    14  15   1  102
element forceBeamColumn 15    15  16   1  102

# =============================================================================
# 滑移模型 Slip Model (零长单元)
# =============================================================================
set P_dyn     [expr -$P_static * $DIF_P]
set uu         1.0
set Vk         [expr $P_dyn * $uu]
set Vc         [expr 0.45 * $Ac * 0.3 * pow($fc1_dyn, 2.0/3.0)]
set Vs         [expr 0.3 * $uu * $As * $fy_dyn]
set Vd         [expr 0.5 * $As * sqrt($fc1_dyn * $fy_dyn)]
set Vmax       [expr $Vk + $Vc + $Vs + $Vd]
set Vplateau   [expr $Vk + $Vs + $Vd]
puts "Vmax = $Vmax, Vplateau = $Vplateau"

set slipshear_matTag 66
set fp1 $Vmax;    set sp1 0.2
set fp2 $Vmax;    set sp2 2.5
set fp3 $Vplateau; set sp3 8.0
set fn1 [expr -$fp1]; set sn1 [expr -$sp1]
set fn2 [expr -$fp2]; set sn2 [expr -$sp2]
set fn3 [expr -$fp3]; set sn3 [expr -$sp3]

puts "$fp1 $sp1 $fp2 $sp2 $fp3 $sp3 $fn1 $sn1 $fn2 $sn2 $fn3 $sn3"

# uniaxialMaterial Hysteretic matTag s1p e1p s2p e2p s3p e3p s1n e1n s2n e2n s3n e3n pinchX pinchY damage1 damage2 beta
uniaxialMaterial Hysteretic $slipshear_matTag \
    $fp1 $sp1  $fp2 $sp2  $fp3 $sp3 \
    $fn1 $sn1  $fn2 $sn2  $fn3 $sn3 \
    1.0 1.0  0.0 0.0  0.0

element zeroLength 36  4 104  -mat $slipshear_matTag  -dir 1

# =============================================================================
# 接触模型 Contact Model (零长单元)
# =============================================================================
uniaxialMaterial Elastic     7   1.2e5
uniaxialMaterial Viscous     8   1.0e5  1.0
uniaxialMaterial ElasticPPGap 9  1.0e6  1.0e20  12

# 弹簧 (Elastic)
element zeroLength 210  5  19  -mat 7  -dir 1
element zeroLength 211  6  20  -mat 7  -dir 1
element zeroLength 212  7  21  -mat 7  -dir 1

# 阻尼 (Viscous)
element zeroLength 995  5  19  -mat 8  -dir 1
element zeroLength 996  6  20  -mat 8  -dir 1
element zeroLength 997  7  21  -mat 8  -dir 1

# 间隙 (ElasticPPGap)
element zeroLength 984  19 29  -mat 9  -dir 1
element zeroLength 985  20 30  -mat 9  -dir 1
element zeroLength 986  21 31  -mat 9  -dir 1

# =============================================================================
# 施加轴压荷载 Axial Load (静力)
# =============================================================================
timeSeries Constant 100
pattern Plain 1 100 {
    load 16  0.0 $P_static 0.0
}

# 静力分析
system BandGeneral
numberer Plain
constraints Plain
test EnergyIncr 1.0e-8 500
algorithm Newton
integrator LoadControl 1.0
analysis Static
analyze 1

puts "Static analysis completed."

# 保持荷载并重置时间
loadConst -time 0.0

# =============================================================================
# 定义记录器 Recorders
# =============================================================================
recorder Node -file "./Column/node_disp.txt"  -time -node 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 -dof 1 disp
recorder Node -file "./Column/node_104.txt"   -time -node 104 -dof 1 disp

# =============================================================================
# 定义冲击荷载 Impact Load (动力)
# =============================================================================
set Mt  [expr $impact_mass * 1000.0 / 3.0]   ;# 单节点冲击质量 (kg)
set V0  $impact_velocity                      ;# 冲击速度 (m/s)
set ts  0.0001                                ;# 冲击力持续时间 (s)
set P   [expr 4.0 * $Mt * $V0 / $ts]          ;# 冲击力 (N)

puts "Impact load P = $P N"

# 矩形脉冲: 0~ts 为 P, 之后为 0
timeSeries Path 101 -time 0.0 $ts -values $P 0.0
pattern Plain 2 101 {
    load 29  1.0  0.0  0.0
    load 30  1.0  0.0  0.0
    load 31  1.0  0.0  0.0
}

# 瞬态动力分析
system BandGeneral
numberer Plain
constraints Plain
test EnergyIncr 1.0e-8 500
algorithm Newton
analysis Transient
integrator Newmark 0.5 0.25
analyze 20000 0.00005

puts "Dynamic analysis completed."

wipe
puts "All analyses completed successfully."
