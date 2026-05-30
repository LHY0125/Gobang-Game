mod commands;

use commands::AppState;

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .manage(AppState::default())
        .invoke_handler(tauri::generate_handler![
            commands::new_game,
            commands::place_piece,
            commands::undo,
            commands::get_board,
            commands::ai_move,
            commands::get_game_state,
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
