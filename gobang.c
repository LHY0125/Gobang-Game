#include "gobang.h"

// 全局变量定义
int BOARD_SIZE = 15;                                           // 实际使用的棋盘尺寸(默认15)
int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE] = {0};               // 棋盘状态存储数组(默认棋盘全空为0)
const int direction[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}}; // 四个方向：向下、向右、右下、左下
Step steps[MAX_STEPS];                                         // 存储所有落子步骤的数组
int step_count = 0;                                            // 当前步数计数器

/**
 * @brief 初始化棋盘为全空状态并重置步数计数器
 * 清空棋盘数组并将所有位置设为EMPTY，同时将step_count重置为0
 */
void empty_board()
{
    // 初始化棋盘状态为全空
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            board[i][j] = EMPTY;
        }
    }
    step_count = 0; // 重置步数计数器
}

/**
 * @brief 打印当前棋盘状态
 * 以可读格式输出棋盘，包括行列号和棋子状态
 * 玩家棋子显示为'x'，AI棋子显示为'○'，空位显示为'·'
 */
void print_board()
{
    // 打印列号（1-BOARD_SIZE显示）
    printf("\n  ");
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        printf("%2d", i + 1);
        if (i + 1 == 9) // 处理列号9和10+的对齐
            printf(" ");
    }
    printf("\n");

    // 逐行打印棋盘内容
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        printf("%2d ", i + 1); // 打印行号（1-BOARD_SIZE）
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == PLAYER)
                printf("x "); // 玩家棋子
            else if (board[i][j] == AI)
                printf("○ "); // AI棋子(使用○显示)
            else
                printf("· "); // 空位
        }
        printf("\n"); // 每行结束换行
    }
}

/**
 * @brief 检查指定位置是否有效且为空
 * @param x 行坐标(0-base)
 * @param y 列坐标(0-base)
 * @return true 位置有效且为空
 * @return false 位置无效或已被占用
 */
bool have_space(int x, int y)
{
    // 校验坐标是否在范围内且该位置为空
    return (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && board[x][y] == EMPTY);
}

/**
 * @brief 执行玩家落子操作
 * @param x 行坐标(0-base)
 * @param y 列坐标(0-base)
 * @return true 落子成功
 * @return false 落子失败(位置无效)
 */
bool player_move(int x, int y)
{
    // 位置无效则返回false
    if (!have_space(x, y))
        return false;

    // 更新棋盘状态
    board[x][y] = PLAYER;
    // 记录落子步骤：玩家标识和坐标
    steps[step_count++] = (Step){PLAYER, x, y};
    return true;
}

/**
 * @brief 计算特定方向上连续同色棋子数量
 * @param x 起始行坐标
 * @param y 起始列坐标
 * @param dx 行方向增量(-1,0,1)
 * @param dy 列方向增量(-1,0,1)
 * @param player 玩家标识(PLAYER/AI)
 * @return DirInfo 包含连续棋子数和方向开放状态的结构体
 * @note 检查正反两个方向，统计连续棋子数并判断端点是否开放
 */
DirInfo count_specific_direction(int x, int y, int dx, int dy, int player)
{
    DirInfo info;
    info.continuous_chess = 1; // 起始位置已经有一个棋子
    info.check_start = false;  // 起点方向是否开放
    info.check_end = false;    // 终点方向是否开放

    // 检查正方向（dx, dy）
    int nx = x + dx, ny = y + dy;
    while (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == player)
    {
        info.continuous_chess++; // 连续棋子计数增加
        nx += dx;                // 沿当前方向前进
        ny += dy;
    }
    // 判断正方向端点是否开放（遇到空位）
    if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE)
        if (board[nx][ny] == EMPTY)
            info.check_end = true;

    // 检查反方向（-dx, -dy）
    nx = x - dx, ny = y - dy;
    while (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[nx][ny] == player)
    {
        info.continuous_chess++; // 连续棋子计数增加
        nx -= dx;                // 沿相反方向前进
        ny -= dy;
    }
    // 判断反方向端点是否开放（遇到空位）
    if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE)
        if (board[nx][ny] == EMPTY)
            info.check_start = true;

    return info;
}

/**
 * @brief 检查特定位置落子后是否形成五连珠获胜
 * @param x 行坐标(0-base)
 * @param y 列坐标(0-base)
 * @param player 玩家标识(PLAYER/AI)
 * @return true 在任意方向形成五连珠
 * @return false 未形成五连珠
 * @note 检查四个方向(水平、垂直、对角线)是否存在连续5个同色棋子
 */
bool check_win(int x, int y, int player)
{
    // 检查四个方向是否存在五连珠
    for (int i = 0; i < 4; i++)
    {
        DirInfo info = count_specific_direction(x, y, direction[i][0], direction[i][1], player);
        if (info.continuous_chess >= 5) // 连续棋子>=5即获胜
            return true;
    }
    return false; // 四个方向都没有五连珠
}

/**
 * @brief 评估特定位置对当前玩家的战略价值
 * @param x 行坐标(0-base)
 * @param y 列坐标(0-base)
 * @param player 玩家标识(PLAYER/AI)
 * @return int 综合评估分数(越高表示位置越好)
 * @note 评分标准:
 * - 活四:100000 冲四:10000 死四:500
 * - 活三:5000 眠三:1000 死三:50
 * - 活二:500 眠二:100 死二:10
 * - 单子:50(开放)/10(半开放)/1(封闭)
 * - 中心位置有额外加成
 */
int evaluate_pos(int x, int y, int player)
{
    // 保存原始值用于还原
    int original = board[x][y];
    // 模拟在该位置落子
    board[x][y] = player;

    int total_score = 0;      // 总分
    int line_scores[4] = {0}; // 四个方向的得分

    // 遍历四个方向进行评估
    for (int i = 0; i < 4; i++)
    {
        int dx = direction[i][0], dy = direction[i][1];
        // 获取当前方向上的棋型信息
        DirInfo info = count_specific_direction(x, y, dx, dy, player);

        // 直接形成五连珠为必胜
        if (info.continuous_chess >= 5)
        {
            board[x][y] = original; // 还原棋盘
            return 1000000;         // 返回最大分
        }

        // 根据连续棋子数评分
        switch (info.continuous_chess)
        {
        case 4:                                     // 四连珠
            if (info.check_start && info.check_end) // 活四(两端开放)
                line_scores[i] = 100000;
            else if (info.check_start || info.check_end) // 冲四(一端开放)
                line_scores[i] = 10000;
            else // 死四(两端封闭)
                line_scores[i] = 500;
            break;

        case 3:                                     // 三连珠
            if (info.check_start && info.check_end) // 活三
                line_scores[i] = 5000;
            else if (info.check_start || info.check_end) // 眠三
                line_scores[i] = 1000;
            else // 死三
                line_scores[i] = 50;
            break;

        case 2:                                     // 二连珠
            if (info.check_start && info.check_end) // 活二
                line_scores[i] = 500;
            else if (info.check_start || info.check_end) // 眠二
                line_scores[i] = 100;
            else // 死二
                line_scores[i] = 10;
            break;

        case 1:                                     // 单子
            if (info.check_start && info.check_end) // 开放位置
                line_scores[i] = 50;
            else if (info.check_start || info.check_end) // 半开放位置
                line_scores[i] = 10;
            else // 封闭位置
                line_scores[i] = 1;
            break;
        }
    }

    // 计算总分（最高方向分+其他方向分加权）
    int max_score = 0;
    int sum_score = 0;
    for (int i = 0; i < 4; i++)
    {
        if (line_scores[i] > max_score)
            max_score = line_scores[i];
        sum_score += line_scores[i];
    }
    total_score = max_score * 10 + sum_score; // 主方向权重更高

    // 位置奖励：越靠近中心分数越高
    int center_x = BOARD_SIZE / 2;
    int center_y = BOARD_SIZE / 2;
    int distance = abs(x - center_x) + abs(y - center_y); // 曼哈顿距离
    int position_bonus = 50 * (BOARD_SIZE - distance);    // 距离中心越近奖励越高

    board[x][y] = original;              // 还原棋盘状态
    return total_score + position_bonus; // 返回总评估分
}

/**
 * @brief 带α-β剪枝的深度优先搜索(极小极大算法实现)
 * @param x 当前行坐标
 * @param y 当前列坐标
 * @param player 当前玩家
 * @param depth 剩余搜索深度
 * @param alpha α值(当前最大值)
 * @param beta β值(当前最小值)
 * @param is_maximizing 是否为极大化玩家(AI)
 * @return int 最佳评估分数
 * @note 算法流程:
 * 1. 检查是否获胜或达到搜索深度
 * 2. 遍历所有可能落子位置
 * 3. 递归评估每个位置的分数
 * 4. 根据is_maximizing选择最大/最小值
 * 5. 使用α-β剪枝优化搜索过程
 */
int dfs(int x, int y, int player, int depth, int alpha, int beta, bool is_maximizing)
{
    // 检查当前落子是否获胜
    if (check_win(x, y, player))
    {
        return (player == AI) ? 1000000 + depth : -1000000 - depth;
    }

    // 达到搜索深度或平局
    if (depth == 0 || step_count >= BOARD_SIZE * BOARD_SIZE)
    {
        return evaluate_pos(x, y, AI) - evaluate_pos(x, y, PLAYER);
    }

    int best_score = is_maximizing ? -1000000 : 1000000;

    // 遍历所有可能落子位置
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] != EMPTY)
                continue;

            // 模拟当前玩家落子
            board[i][j] = player;
            step_count++;

            // 递归搜索(切换玩家和搜索深度)
            int current_score = dfs(i, j, (player == AI) ? PLAYER : AI, depth - 1, alpha, beta, !is_maximizing);

            // 撤销落子
            board[i][j] = EMPTY;
            step_count--;

            // 极大值玩家(AI)逻辑
            if (is_maximizing)
            {
                best_score = (current_score > best_score) ? current_score : best_score;
                alpha = (best_score > alpha) ? best_score : alpha;
                // α剪枝
                if (beta <= alpha)
                {
                    break;
                }
            }
            // 极小值玩家(人类)逻辑
            else
            {
                best_score = (current_score < best_score) ? current_score : best_score;
                beta = (best_score < beta) ? best_score : beta;
                // β剪枝
                if (beta <= alpha)
                {
                    break;
                }
            }
        }
        if ((is_maximizing && best_score >= beta) || (!is_maximizing && best_score <= alpha))
        {
            break; // 提前退出外层循环
        }
    }

    return best_score;
}

/**
 * @brief AI决策主函数，使用评估函数和搜索算法选择最佳落子位置
 * @note 采用两阶段决策逻辑：
 * 1. 防御阶段：检查并阻止玩家即将获胜的位置（活四、冲四、活三）
 * 2. 进攻阶段：若无紧急防御需求，使用DFS评估选择最佳进攻位置
 * @note 实现细节：
 * - 优先处理玩家活四、冲四等危险局面
 * - 步数>10时缩小搜索范围到已有棋子附近2格
 * - 使用中心位置优先策略
 */
void ai_move(int depth)
{
    // 1. 首先检查是否需要阻止玩家的四子连棋或三子活棋
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] != EMPTY)
                continue;

            // 模拟玩家在此位置落子
            board[i][j] = PLAYER;
            bool need_block = false;

            // 检查四个方向
            for (int k = 0; k < 4; k++)
            {
                DirInfo info = count_specific_direction(i, j, direction[k][0], direction[k][1], PLAYER);

                // 如果玩家能形成四子连棋且至少一端开放
                if (info.continuous_chess >= 4 && (info.check_start || info.check_end))
                {
                    need_block = true;
                    break;
                }

                // 如果玩家能形成三子活棋且两端开放
                if (info.continuous_chess == 3 && info.check_start && info.check_end)
                {
                    need_block = true;
                    break;
                }
            }

            board[i][j] = EMPTY; // 恢复棋盘

            if (need_block)
            {
                // 必须在此位置落子阻止
                board[i][j] = AI;
                steps[step_count++] = (Step){AI, i, j};
                printf("AI落子(%d, %d)\n", i + 1, j + 1);
                return;
            }
        }
    }

    // 2. 如果没有需要立即阻止的情况，则正常评估
    int best_score = -1000000;
    int best_x = -1, best_y = -1;

    // 遍历棋盘所有空位
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] != EMPTY)
                continue;

            // 只考虑已有棋子附近(2格范围内)
            bool has_nearby_stone = false;
            for (int di = -2; di <= 2; di++)
            {
                for (int dj = -2; dj <= 2; dj++)
                {
                    int ni = i + di;
                    int nj = j + dj;
                    if (ni >= 0 && ni < BOARD_SIZE &&
                        nj >= 0 && nj < BOARD_SIZE)
                    {
                        if (board[ni][nj] != EMPTY)
                        {
                            has_nearby_stone = true;
                            break;
                        }
                    }
                }
                if (has_nearby_stone)
                    break;
            }
            if (!has_nearby_stone && step_count > 10)
                continue;

            // 模拟AI落子
            board[i][j] = AI;
            int current_score = dfs(i, j, PLAYER, depth, -1000000, 1000000, false);
            board[i][j] = EMPTY;

            // 更新最佳位置
            if (current_score > best_score)
            {
                best_score = current_score;
                best_x = i;
                best_y = j;
            }
        }
    }

    // 执行最佳落子
    if (best_x != -1 && best_y != -1)
    {
        board[best_x][best_y] = AI;
        steps[step_count++] = (Step){AI, best_x, best_y};
        printf("AI落子(%d, %d)\n", best_x + 1, best_y + 1);
    }
}

/**
 * @brief 复盘游戏过程，逐步重现所有落子步骤
 * @note 实现逻辑：
 * 1. 创建临时棋盘用于复盘展示
 * 2. 按步数顺序逐步重现每个落子
 * 3. 每步显示当前棋盘状态和落子信息
 * 4. 通过用户按Enter键控制步骤前进
 * 5. 显示1-based坐标方便用户查看
 */
void review_process()
{
    printf("\n===== 复盘记录(总步数：%d) =====\n", step_count);
    // 清空输入缓冲区
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;

    // 创建临时复盘棋盘
    int temp_board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    memset(temp_board, EMPTY, sizeof(temp_board)); // 初始化为空棋盘

    // 逐步重现游戏过程
    for (int i = 0; i < step_count; i++)
    {
        Step s = steps[i];               // 获取当前步骤
        temp_board[s.x][s.y] = s.player; // 在临时棋盘上落子

        // 打印当前步骤信息
        printf("\n===== 五子棋人机对战(%dX%d棋盘) =====", BOARD_SIZE, BOARD_SIZE);
        printf("\n    第%d步/%d步: %s 落子于(%d, %d)\n",
               i + 1, step_count,
               (s.player == PLAYER) ? "玩家" : "AI", // 三目运算符选择显示文本
               s.x + 1, s.y + 1);                    // 显示1-base坐标

        // 打印当前复盘棋盘
        printf("  ");
        for (int col = 0; col < BOARD_SIZE; col++)
            printf("%2d", col + 1); // 列号
        printf("\n");

        for (int row = 0; row < BOARD_SIZE; row++)
        {
            printf("%2d ", row + 1); // 行号
            for (int col = 0; col < BOARD_SIZE; col++)
            {
                if (temp_board[row][col] == PLAYER)
                    printf("x ");
                else if (temp_board[row][col] == AI)
                    printf("○ ");
                else
                    printf("· ");
            }
            printf("\n"); // 行结束换行
        }

        // 如果不是最后一步，等待用户按键继续
        if (i < step_count - 1)
        {
            printf("\n按Enter继续下一步...");
            while (getchar() != '\n')
                ; // 等待回车
        }
    }
    printf("\n复盘结束！按Enter返回...");
    getchar(); // 等待用户按键
}