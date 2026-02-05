module types;

MoveResult::MoveResult(bool okValue, bool gameOverValue, PlayerId winnerValue) : ok{okValue},
                                                                                 gameOver{gameOverValue},
                                                                                 winner{winnerValue} {}
