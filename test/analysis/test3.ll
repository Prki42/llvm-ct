define i32 @test_clean_logic(i32 %secret_val) {
entry:
  ; --- 1. CLEAR BRANCH ---
  ; Compares just 1 > 0
  %cmp_const = icmp sgt i32 1, 0
  br i1 %cmp_const, label %clean_branch, label %end

clean_branch:
  ret i32 1

  ; --- 2. MARKED BRANCH (DEPENDS ON %secret_val) ---
  %cmp_secret = icmp eq i32 %secret_val, 42
  br i1 %cmp_secret, label %secret_branch, label %end

secret_branch:
  ret i32 2

end:
  ret i32 0
}