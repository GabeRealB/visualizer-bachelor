
// SEQ_CLOCK-Schleifen
zeichne_würfel("computation, 1. würfel")
for (int lcl_til_id_1 = 1; lcl_til_id_1 <= num_lcl_1; ++lcl_til_id_1) {
    for (int lcl_til_id_2 = 1; lcl_til_id_2 <= num_lcl_2; ++lcl_til_id_2) {
        for (int lcl_til_id_3 = 1; lcl_til_id_3 <= num_lcl_3; ++lcl_til_id_3) {

            zeichne_würfel("computation, 2. würfel")

            for (int prv_til_id_1 = 1; prv_til_id_1 <= num_prv_1; ++prv_til_id_1) {
                for (int prv_til_id_2 = 1; prv_til_id_2 <= num_prv_2; ++prv_til_id_2) {
                    for (int prv_til_id_3 = 1; prv_til_id_3 <= num_prv_3; ++prv_til_id_3) {
                        for (int seq_til_id_1 = 1; seq_til_id_1 <= num_seq_1; ++seq_til_id_1) {
                            for (int seq_til_id_2 = 1; seq_til_id_2 <= num_seq_2; ++seq_til_id_2) {
                                for (int seq_til_id_3 = 1; seq_til_id_3 <= num_seq_3; ++seq_til_id_3) {

                                    // PAR_CLOCK-Schleifen
                                    for (int wg_id_1 = 1; wg_id_1 <= num_wg_1; ++wg_id_1) {
                                        for (int wg_id_2 = 1; wg_id_2 <= num_wg_2; ++wg_id_2) {
                                            for (int wg_id_3 = 1; wg_id_3 <= num_wg_3; ++wg_id_3) {

                                                zeichne_würfel("input mda, 2. würfel")
                                                zeichne_würfel("computation, 5. würfel")

                                                for (int wi_id_1 = 1; wi_id_1 <= num_wi_1; ++wi_id_1) {
                                                    for (int wi_id_2 = 1; wi_id_2 <= num_wi_2; ++wi_id_2) {
                                                        for (int wi_id_3 = 1; wi_id_3 <= num_wi_3; ++wi_id_3) {

                                                            zeichne_würfel("computation, 6. würfel")

                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    sleep(1)

                                    // lösche alle Würfel, die in den PAR_CLOCK-Schleifen gezeichnet wurden

                                }
                            }
                        }
                    }
                }
            }

            lösche_würfel("computation, 2. würfel")

        }
    }
}
lösche_würfel("computation, 1. würfel")