import { useState, useEffect } from 'react';
import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import { MIN_BOARD_SIZE, MAX_BOARD_SIZE } from '../../core/constants';
import type { Color, GameConfig } from '../../core/types';

const LS_LLM_ENDPOINT = 'gobang_llm_endpoint';
const LS_LLM_API_KEY = 'gobang_llm_api_key';
const LS_LLM_MODEL = 'gobang_llm_model';

function loadLlmConfig() {
  return {
    endpoint: localStorage.getItem(LS_LLM_ENDPOINT) || 'https://api.openai.com/v1/chat/completions',
    apiKey: localStorage.getItem(LS_LLM_API_KEY) || '',
    model: localStorage.getItem(LS_LLM_MODEL) || 'gpt-4o-mini',
  };
}

interface Props {
  onBack: () => void;
  onStart: () => void;
}

type AiType = 'alpha-beta' | 'llm';

export default function AiGameSetup({ onBack, onStart }: Props) {
  const { t } = useTranslation();
  const startGame = useGameStore((s) => s.startGame);
  const saved = loadLlmConfig();
  const [boardSize, setBoardSize] = useState(15);
  const [aiType, setAiType] = useState<AiType>('alpha-beta');
  const [difficulty, setDifficulty] = useState(3);
  const [playerColor, setPlayerColor] = useState<Color>('Black');
  const [useForbidden, setUseForbidden] = useState(true);
  const [llmEndpoint, setLlmEndpoint] = useState(saved.endpoint);
  const [llmApiKey, setLlmApiKey] = useState(saved.apiKey);
  const [llmModel, setLlmModel] = useState(saved.model);

  // LLM 配置变更时自动保存到 localStorage
  useEffect(() => { localStorage.setItem(LS_LLM_ENDPOINT, llmEndpoint); }, [llmEndpoint]);
  useEffect(() => { localStorage.setItem(LS_LLM_API_KEY, llmApiKey); }, [llmApiKey]);
  useEffect(() => { localStorage.setItem(LS_LLM_MODEL, llmModel); }, [llmModel]);

  const handleStart = async () => {
    const config: GameConfig = {
      boardSize,
      useForbiddenRules: useForbidden,
      useTimer: false,
      timeLimitSecs: 60,
      aiDifficulty: difficulty,
      playerColor,
      isServer: false,
      remoteAddress: '',
      useLlm: aiType === 'llm',
      llmEndpoint,
      llmApiKey,
      llmModel,
    };
    await startGame('VsAi', config);
    onStart();
  };

  return (
    <div className="setup-panel">
      <h2>{t('menu.ai_game')}</h2>

      <label>
        {t('ai_setup.ai_type')}:
        <select value={aiType} onChange={(e) => setAiType(e.target.value as AiType)}>
          <option value="alpha-beta">{t('ai_setup.alpha_beta')}</option>
          <option value="llm">{t('ai_setup.llm')}</option>
        </select>
      </label>

      {aiType === 'llm' && (
        <>
          <label>
            {t('ai_setup.llm_endpoint')}:
            <input
              type="text"
              value={llmEndpoint}
              onChange={(e) => setLlmEndpoint(e.target.value)}
              placeholder={t('ai_setup.llm_endpoint_placeholder')}
            />
          </label>
          <label>
            {t('ai_setup.llm_api_key')}:
            <input
              type="password"
              value={llmApiKey}
              onChange={(e) => setLlmApiKey(e.target.value)}
              placeholder="sk-..."
            />
          </label>
          <label>
            {t('ai_setup.llm_model')}:
            <input
              type="text"
              value={llmModel}
              onChange={(e) => setLlmModel(e.target.value)}
              placeholder={t('ai_setup.llm_model_placeholder')}
            />
          </label>
        </>
      )}

      <label>
        {t('settings.board_size')}:
        <select value={boardSize} onChange={(e) => setBoardSize(Number(e.target.value))}>
          {Array.from({ length: MAX_BOARD_SIZE - MIN_BOARD_SIZE + 1 }, (_, i) => MIN_BOARD_SIZE + i).map((s) => (
            <option key={s} value={s}>{s}&times;{s}</option>
          ))}
        </select>
      </label>

      {aiType === 'alpha-beta' && (
        <label>
          {t('settings.difficulty')}:
          <select value={difficulty} onChange={(e) => setDifficulty(Number(e.target.value))}>
            {[1, 2, 3, 4, 5].map((d) => (
              <option key={d} value={d}>{d}</option>
            ))}
          </select>
        </label>
      )}

      <label>
        {t('ai_setup.first_player')}:
        <select value={playerColor} onChange={(e) => setPlayerColor(e.target.value as Color)}>
          <option value="Black">{t('ai_setup.black_first')}</option>
          <option value="White">{t('ai_setup.white_second')}</option>
        </select>
      </label>

      <label>
        <input type="checkbox" checked={useForbidden} onChange={(e) => setUseForbidden(e.target.checked)} />
        {t('settings.forbidden_rules')}
      </label>

      <div className="setup-actions">
        <button onClick={handleStart}>{t('game.new_game')}</button>
        <button onClick={onBack}>{t('common.back')}</button>
      </div>
    </div>
  );
}
