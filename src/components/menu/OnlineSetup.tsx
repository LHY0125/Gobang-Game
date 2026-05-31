import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import { useGameStore } from '../../store/gameStore';
import { invoke } from '@tauri-apps/api/core';
import { listen } from '@tauri-apps/api/event';
import { MIN_BOARD_SIZE, MAX_BOARD_SIZE } from '../../core/constants';
import type { GameConfig } from '../../core/types';

interface Props { onBack: () => void; onStart: () => void; }

export default function OnlineSetup({ onBack, onStart }: Props) {
  const { t } = useTranslation();
  const startGame = useGameStore((s) => s.startGame);
  const [boardSize, setBoardSize] = useState(15);
  const [ip, setIp] = useState('');
  const [myAddress, setMyAddress] = useState('');
  const [isHosting, setIsHosting] = useState(false);

  const handleHost = async () => {
    try {
      const port: number = await invoke('host_game', { port: 0 });
      setMyAddress(`127.0.0.1:${port}`);
      setIsHosting(true);

      const unlisten = await listen<string>('connection-status', async (event) => {
        if (event.payload === 'connected') {
          unlisten();
          const config: GameConfig = {
            boardSize, useForbiddenRules: true, useTimer: false,
            timeLimitSecs: 60, aiDifficulty: 3, playerColor: 'Black',
            isServer: true, remoteAddress: '', hostPort: port,
            useLlm: false, llmEndpoint: '', llmApiKey: '', llmModel: '',
          };
          await startGame('Online', config);
          onStart();
        }
      });
    } catch (e) {
      alert('创建房间失败: ' + e);
    }
  };

  const handleJoin = async () => {
    try {
      const [_, portStr] = ip.split(':');
      const port = parseInt(portStr) || 0;
      const config: GameConfig = {
        boardSize, useForbiddenRules: true, useTimer: false,
        timeLimitSecs: 60, aiDifficulty: 3, playerColor: 'White',
        isServer: false, remoteAddress: ip, hostPort: port,
        useLlm: false, llmEndpoint: '', llmApiKey: '', llmModel: '',
      };
      await startGame('Online', config);
      await invoke('join_game', { address: ip });
      onStart();
    } catch (e) {
      alert('加入房间失败: ' + e);
    }
  };

  if (isHosting) {
    return (
      <div className="setup-panel">
        <h2>{t('menu.online_game')}</h2>
        <p style={{ fontSize: 18 }}>等待对手加入...</p>
        <p style={{ fontSize: 24, fontFamily: 'monospace', background: '#F5DEB3', color: '#3C2415', padding: '8px 16px', borderRadius: 4 }}>
          {myAddress}
        </p>
        <p style={{ fontSize: 14, opacity: 0.7 }}>将此地址发给对手</p>
        <button onClick={onBack}>{t('common.back')}</button>
      </div>
    );
  }

  return (
    <div className="setup-panel">
      <h2>{t('menu.online_game')}</h2>
      <label>
        {t('settings.board_size')}:
        <select value={boardSize} onChange={(e) => setBoardSize(Number(e.target.value))}>
          {Array.from({ length: MAX_BOARD_SIZE - MIN_BOARD_SIZE + 1 }, (_, i) => MIN_BOARD_SIZE + i).map((s) => (
            <option key={s} value={s}>{s}×{s}</option>
          ))}
        </select>
      </label>
      <button onClick={handleHost}>{t('menu.host_room')}</button>
      <div style={{ display: 'flex', gap: 8, marginTop: 12 }}>
        <input value={ip} onChange={(e) => setIp(e.target.value)} placeholder={t('menu.ip_placeholder')} />
        <button onClick={handleJoin} disabled={!ip}>{t('menu.join_room')}</button>
      </div>
      <button onClick={onBack} style={{ marginTop: 12 }}>{t('common.back')}</button>
    </div>
  );
}
