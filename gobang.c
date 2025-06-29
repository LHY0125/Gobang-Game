#include "gobang.h"
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

// 全局变量定义
int BOARD_SIZE = 15;                                           // 实际使用的棋盘尺寸(默认15)
int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE] = {0};               // 棋盘状态存储数组(默认棋盘全空为0)
const int direction[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}}; // 四个方向：向下、向右、右下、左下
Step steps[MAX_STEPS];                                         // 存储所有落子步骤的数组
int step_count = 0;                                            // 当前步数计数器
bool use_forbidden_moves = false;                              // 默认不启用禁手规则
int use_timer = 0;                                             // 默认不启用计时器
int time_limit = 30;                                           // 默认时间限制为30秒

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
 * @brief 玩家落子操作
 *
 * @param player1
 * @param player2
 * @return int player1 or player2
 */
void setup_board_size()
{
    printf("通常棋盘大小分为休闲棋盘(13X13)、标准棋盘(15X15)和特殊棋盘(19X19)\n");
    char prompt[100];
    sprintf(prompt, "请输入棋盘大小(5~%d)(默认为标准棋盘):\n", MAX_BOARD_SIZE);
    BOARD_SIZE = get_integer_input(prompt, 5, MAX_BOARD_SIZE);
}

/**
 * @brief Set the up game options object
 * 配置游戏选项，包括禁手规则、计时器和时间限制
 */
void setup_game_options()
{
    use_forbidden_moves = get_integer_input("是否启用禁手规则 (1-是, 0-否): ", 0, 1);

    use_timer = get_integer_input("是否启用计时器 (1-是, 0-否): ", 0, 1);
    if (use_timer)
    {
        time_limit = get_integer_input("请输入每回合的时间限制 (分钟): ", 1, 60) * 60;
    }
}

/**
 * @brief 确定先手玩家
 *
 * @param player1
 * @param player2
 * @return int player1 or player2
 */
int determine_first_player(int player1, int player2)
{
    char prompt[100];
    sprintf(prompt, "请选择先手方 (1 for Player %d, 2 for Player %d): ", player1, player2);
    int first_player_choice = get_integer_input(prompt, 1, 2);
    if (first_player_choice == 1)
    {
        return player1;
    }
    else
    {
        return player2;
    }
}

/**
 * @brief 检查是否为禁手
 *
 * @param x
 * @param y
 * @param player
 * @return true
 * @return false
 */
bool is_forbidden_move(int x, int y, int player)
{
    if (!use_forbidden_moves)
    {
        return false;
    }
    if (player != PLAYER && player != PLAYER3)
    {
        return false;
    }

    board[x][y] = player;

    int three_count = 0;
    int four_count = 0;

    for (int i = 0; i < 4; i++)
    {
        DirInfo info = count_specific_direction(x, y, direction[i][0], direction[i][1], player);

        if (info.continuous_chess > 5)
        {
            board[x][y] = EMPTY;
            return true; // 长连禁手
        }
        if (info.continuous_chess == 3 && info.check_start && info.check_end)
        {
            three_count++;
        }
        if (info.continuous_chess == 4 && (info.check_start || info.check_end))
        {
            four_count++;
        }
    }

    board[x][y] = EMPTY;

    if (three_count >= 2 || four_count >= 2)
    {
        return true; // 三三或四四禁手
    }

    return false;
}

/**
 * @brief 执行玩家落子操作
 * @param x 行坐标(0-base)
 * @param y 列坐标(0-base)
 * @return true 落子成功
 * @return false 落子失败(位置无效)
 */
bool player_move(int x, int y, int player)
{
    // 位置无效则返回false
    if (!have_space(x, y))
        return false;

    if (is_forbidden_move(x, y, player))
    {
        printf("禁手！请选择其他位置。\n");
        return false;
    }

    // 更新棋盘状态
    board[x][y] = player;
    // 记录落子步骤：玩家标识和坐标
    steps[step_count++] = (Step){player, x, y};
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

bool check_win(int x, int y, int player)
{
    // 检查四个方向是否存在五连珠
    for (int i = 0; i < 4; i++)
    {
        DirInfo info = count_specific_direction(x, y, direction[i][0], direction[i][1], player);
        if (info.continuous_chess >= 5) // 连续棋子>=5即获胜
        {
            return true;
        }
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
int get_integer_input(const char *prompt, int min, int max)
{
    int value;
    int result;
    char ch;

    while (1)
    {
        printf("%s", prompt);
        result = scanf("%d", &value);

        if (result == 1 && value >= min && value <= max)
        {
            // 清除输入缓冲区中剩余的字符
            while ((ch = getchar()) != '\n' && ch != EOF);
            return value;
        }
        else
        {
            // 清除无效输入
            while ((ch = getchar()) != '\n' && ch != EOF);
            printf("输入无效，请输入一个介于 %d 和 %d 之间的整数。\n", min, max);
        }
    }
}

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
 * @brief 复盘游戏全过程并展示评分
 * @note 实现流程:
 * 1. 初始化临时复盘棋盘
 * 2. 按步数顺序逐步重现每个落子
 * 3. 每步显示:
 *    - 当前步数/总步数
 *    - 落子方(玩家/AI)
 *    - 落子位置(1-based坐标)
 *    - 当前棋盘状态
 * 4. 通过用户按Enter键控制步骤前进
 * 5. 复盘结束后自动进入评分环节:
 *    - 评估双方表现
 *    - 显示得分
 *    - 评选MVP
 * @note 技术细节:
 * - 使用独立临时棋盘避免影响主游戏状态
 * - 坐标显示转换为1-based方便用户理解
 * - 包含输入缓冲区清理防止意外输入
 * - 评分环节调用evaluate_performance()函数
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
    printf("\n复盘结束！按Enter查看评分...");
    getchar(); // 等待用户按键

    // 评估双方表现
    printf("\n===== 对局评分 =====\n");
    int player_score = evaluate_performance(PLAYER);
    int ai_score = evaluate_performance(AI);

    double sum_score = (long double)player_score + (long double)ai_score;

    if (sum_score > 0)
    {
        printf("玩家得分: %d, 占比: %.2f%%\n",
               player_score, (double)player_score * 100.0 / sum_score);
        printf("AI得分: %d, 占比: %.2f%%\n",
               ai_score, (double)ai_score * 100.0 / sum_score);
    }
    else
    {
        printf("玩家得分: %d\n", player_score);
        printf("AI得分: %d\n", ai_score);
        printf("注: 双方得分均为0，无法计算占比\n");
    }

    // 评选MVP
    if (player_score > ai_score)
    {
        printf("\nMVP: 玩家 (领先 %d 分)\n", player_score - ai_score);
    }
    else if (ai_score > player_score)
    {
        printf("\nMVP: AI (领先 %d 分)\n", ai_score - player_score);
    }
    else
    {
        printf("\n双方势均力敌！\n");
    }

    getchar();
}

/**
 * @brief 悔棋功能实现
 * @param steps_to_undo 要撤销的步数
 * @return true 悔棋成功
 * @return false 悔棋失败(步数不足)
 */
/**
 * @brief 处理游戏结束后的记录保存
 */
void handle_save_record()
{
    int save_choice = 0;
    printf("===== 游戏结束 =====\n");
    printf("是否保存游戏记录? (1-是, 0-否): ");
    scanf("%d", &save_choice);

    if (save_choice == 1)
    {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char filename[256];
        strftime(filename, sizeof(filename), "%Y%m%d_%H%M%S.txt", t);

        int save_status = save_game_to_file(filename);
        switch (save_status)
        {
        case 0: // 成功
            printf("\n游戏记录已成功保存至: %s\n", filename);
            printf("您可以使用以下命令进行复盘: .\\五子棋.exe -l %s\n", filename);
            break;
        case 1: // 目录创建失败
            printf("\n游戏记录保存失败: 无法创建 'records' 目录。\n");
            printf("请检查程序是否具有足够的写入权限或磁盘空间是否充足。\n");
            break;
        case 2: // 文件打开失败
            printf("\n游戏记录保存失败: 无法在路径 '%s' 创建文件。\n", filename);
            printf("请检查路径是否有效以及程序是否具有写入权限。\n");
            break;
        case 3: // 文件写入失败
            printf("\n游戏记录保存失败: 写入文件时发生错误。\n");
            printf("请检查磁盘空间是否已满。\n");
            break;
        default:
            printf("\n游戏记录保存失败: 发生未知错误。\n");
            break;
        }
    }
}

bool return_move(int steps_to_undo)
{
    if (step_count < steps_to_undo)
    {
        return false;
    }

    for (int i = 0; i < steps_to_undo; i++)
    {
        step_count--;
        board[steps[step_count].x][steps[step_count].y] = EMPTY;
    }

    return true;
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
/**
 * @brief 评估玩家在整盘棋局中的表现
 * @param player 要评估的玩家(PLAYER/AI)
 * @return int 总分(已考虑方向重复计算)
 * @note 评分标准:
 * - 五连:1000000
 * - 活四:100000 冲四:10000 死四:500
 * - 活三:5000 眠三:1000 死三:50
 * - 活二:500 眠二:100 死二:10
 * - 开放单子:50 半开放单子:10 封闭单子:1
 * @note 实现细节:
 * 1. 遍历棋盘所有位置
 * 2. 对每个棋子检查四个方向
 * 3. 统计所有连子情况并评分
 * 4. 最终分数除以4(消除方向重复计算影响)
 */
int evaluate_performance(int player)
{
    int total_score = 0;

    // 遍历棋盘所有位置
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] != player)
                continue;

            // 检查四个方向
            for (int k = 0; k < 4; k++)
            {
                DirInfo info = count_specific_direction(i, j, direction[k][0], direction[k][1], player);

                // 根据连子数评分
                switch (info.continuous_chess)
                {
                case 5:
                    total_score += 1000;
                    break; // 五连
                case 4:
                    if (info.check_start && info.check_end)
                        total_score += 1000; // 活四
                    else if (info.check_start || info.check_end)
                        total_score += 500; // 冲四
                    else
                        total_score += 200; // 死四
                    break;
                case 3:
                    if (info.check_start && info.check_end)
                        total_score += 200; // 活三
                    else if (info.check_start || info.check_end)
                        total_score += 100; // 眠三
                    else
                        total_score += 20; // 死三
                    break;
                case 2:
                    if (info.check_start && info.check_end)
                        total_score += 100; // 活二
                    else if (info.check_start || info.check_end)
                        total_score += 50; // 眠二
                    else
                        total_score += 10; // 死二
                    break;
                case 1:
                    if (info.check_start && info.check_end)
                        total_score += 20; // 开放单子
                    else if (info.check_start || info.check_end)
                        total_score += 10; // 半开放单子
                    else
                        total_score += 1; // 封闭单子
                    break;
                }
            }
        }
    }
    return total_score / 4; // 每个方向都计算了，需要除以4
}

/**
 * @brief 将当前游戏记录保存到文件
 * @param filename 要保存的文件名
 * @return int 错误码:
 *   0: 成功
 *   1: 目录创建失败
 *   2: 文件打开失败
 *   3: 文件写入失败
 */
int save_game_to_file(const char *filename)
{
    // 创建records目录(如果不存在)
    struct stat st = {0};
    if (stat("records", &st) == -1)
    {
        if (mkdir("records") != 0)
        {
            // 检查是否目录已存在(多线程情况下可能被其他线程创建)
            if (stat("records", &st) == -1)
            {
#ifdef _WIN32
                printf("错误：无法创建records目录\n");
                printf("可能原因：\n");
                printf("1. 没有写入权限 - 请尝试以管理员身份运行\n");
                printf("2. 防病毒软件阻止 - 请检查安全软件设置\n");
                printf("3. 路径无效 - 请检查工作目录\n");
#else
                perror("创建目录失败");
#endif
                return 1; // 目录创建失败
            }
        }
    }

    // 打开文件
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "records/%s", filename);
    FILE *file = fopen(fullpath, "w");
    if (!file)
    {
        return 2; // 文件打开失败
    }

    // 写入棋盘大小
    if (fprintf(file, "%d\n", BOARD_SIZE) < 0)
    {
        fclose(file);
        return 3; // 文件写入失败
    }

    // 写入所有落子步骤
    for (int i = 0; i < step_count; i++)
    {
        if (fprintf(file, "%d %d %d\n", steps[i].player, steps[i].x, steps[i].y) < 0)
        {
            fclose(file);
            return 3; // 文件写入失败
        }
    }

    if (fclose(file) != 0)
    {
        return 3; // 文件关闭/写入失败
    }

    return 0; // 成功
}

/**
 * @brief 从文件加载游戏记录
 * @param filename 要加载的文件名
 * @return true 加载成功
 * @return false 加载失败
 */
bool load_game_from_file(const char *filename)
{
    // 打开文件
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "records/%s", filename);
    FILE *file = fopen(fullpath, "r");
    if (!file)
    {
        return false;
    }

    // 读取棋盘大小
    int size;
    if (fscanf(file, "%d", &size) != 1 || size < 5 || size > MAX_BOARD_SIZE)
    {
        fclose(file);
        return false;
    }

    // 初始化棋盘
    BOARD_SIZE = size;
    empty_board();

    // 读取并重放所有落子步骤
    int player, x, y;
    while (fscanf(file, "%d %d %d", &player, &x, &y) == 3)
    {
        if (player != PLAYER && player != AI)
        {
            fclose(file);
            return false;
        }
        if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE)
        {
            fclose(file);
            return false;
        }
        board[x][y] = player;
        steps[step_count++] = (Step){player, x, y};
    }

    fclose(file);
    return true;
}