// 前进: 传入 移动距离( 正数 )，当前点下标，当前点已移动距离，回填坐标 & 角度
// 返回是否已移动到终点( isLoop == false )
bool Forward(float total, uint32_t& i, float& d, Point& pos, float& a) const;

// 后退: 传入 移动距离( 正数 )，当前点下标，当前点剩余距离，回填坐标 & 角度
// 返回是否已移动到起点( isLoop == false )
bool Backward(float total, uint32_t& i, float& d, Point& pos, float& a) const;

// 获取起点的数据
void Begin(uint32_t& i, float& d, Point& pos, float& a) const;

// 获取终点的数据
void End(uint32_t& i, float& d, Point& pos, float& a) const;

// 针对手工填充了坐标的数据，填充 角度 和 距离
void FillDA();

// 创建一个曲线连接途经点的 pathway
static xx::Shared<Pathway> MakeCurve(bool const& isLoop, std::vector<CurvePoint> const& ps);
